#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include "tbi/screen.h"
#include "tbi/win.h"
#include "dir.h"
#include "fs.h"
#include "panel.h"

#define SIZE_COLS 7
#define MODIFYTIME_COLS 12
#define MONTH_LENGTH 3

#define HIGHLIGHT_LINE_AS_CURRENT(panel, line) \
	highlight_line(panel, line, -1, EFI_CYAN)
#define UNHIGHLIGHT_LINE_AS_CURRENT(panel, line) \
	highlight_line(panel, line, -1, EFI_BLACK)
#define HIGHLIGHT_LINE_AS_MARK(panel, line) \
	highlight_line(panel, line, EFI_YELLOW, -1)
#define UNHIGHLIGHT_LINE_AS_MARK(panel, line) \
	highlight_line(panel, line, EFI_LIGHTGRAY, -1)

STATIC CONST CHAR16 *get_month_string(UINT8 month)
{
	switch(month) {
		case 1: return L"jan";
		case 2: return L"feb";
		case 3: return L"mar";
		case 4: return L"apr";
		case 5: return L"may";
		case 6: return L"jun";
		case 7: return L"jul";
		case 8: return L"aug";
		case 9: return L"sep";
		case 10: return L"oct";
		case 11: return L"nov";
		case 12: return L"dec";
		default: break;
	}

	return L"xxx";
}

STATIC VOID clear_list_line(struct panel_ctx *p, UINTN line)
{
	mvwprintf(p->wlist, 0, line, L"%-*s%c%*s%c%*s",
		p->name_cols, L"", BOXDRAW_VERTICAL,
		SIZE_COLS, L"", BOXDRAW_VERTICAL,
		MODIFYTIME_COLS, L"");
}

STATIC VOID display_fs(struct panel_ctx *p, UINTN start_index)
{
	UINTN line, i;

	for(line = 0, i = start_index - 1; line < p->list_lines; line++, i++)
	{
		if(i >= p->fsa->len) {
			clear_list_line(p, line);
			continue;
		}

		mvwprintf(p->wlist, 0, line, L"%-*s%c%*s%c%*s",
			p->name_cols, p->fsa->full_name[i], BOXDRAW_VERTICAL,
			SIZE_COLS, L"<fsys>", BOXDRAW_VERTICAL,
			MODIFYTIME_COLS, L"");
	}
}

STATIC VOID display_files(struct panel_ctx *p, UINTN start_index)
{
	UINTN i;
	EFI_SHELL_FILE_INFO *list_head, *node;
	EFI_TIME cur_time, *mod_time;
	BOOLEAN directory;

	gRT->GetTime(&cur_time, NULL);
	list_head = p->dirs->list_head;

	node = dirl_getn(p->dirs, start_index);
	for(i = 0; i <= p->list_lines; i++)
	{
		if(IsNull(&list_head->Link, &node->Link) || 
				((start_index + i) > p->dirs->len)) {
			clear_list_line(p, i);
			continue;
		}

		mod_time = &(node->Info->ModificationTime);
		directory = FALSE;
		if((node->Info->Attribute & EFI_FILE_DIRECTORY) != 0)
			directory = TRUE;

		// mvwprintf doesn't support "-*.*s" :(

		// column "name"
		mvwprintf(p->wlist, 0, i, L"%c%-*s", directory ? L'/' : L' ',
			p->name_cols - 1, node->FileName);

		// column "size"
		if(directory)
			mvwprintf(p->wlist, p->name_cols, i, L"%c%*.*s%c", BOXDRAW_VERTICAL,
				SIZE_COLS, SIZE_COLS, L"<dir>");
		else
			mvwprintf(p->wlist, p->name_cols, i, L"%c%*d%c", BOXDRAW_VERTICAL,
				SIZE_COLS, node->Info->FileSize);

		// column "modify time"
		mvwprintf(p->wlist, p->wlist->width - MODIFYTIME_COLS - 1, i, 
			L"%c%*s %02u ", BOXDRAW_VERTICAL, MONTH_LENGTH,
			get_month_string(mod_time->Month), mod_time->Day);
		if(cur_time.Year == mod_time->Year)
			mvwprintf(p->wlist, p->wlist->width - MODIFYTIME_COLS + 7, i,
				L"%02u:%02u", mod_time->Hour, mod_time->Minute);
		else
			mvwprintf(p->wlist, p->wlist->width - MODIFYTIME_COLS + 7, i,
				L"%5u", mod_time->Year);

		node = (EFI_SHELL_FILE_INFO *)GetNextNode(&list_head->Link, &node->Link);
	}
}

STATIC VOID highlight_line(struct panel_ctx *p, UINTN line, INT32 fg, INT32 bg)
{
	CHAR16 *str;
	INT32 attr;
	UINTN ui_line;

	ui_line = (line-1) % p->list_lines;
	attr = p->wlist->attr[ui_line][0];
	if(fg < 0x0)
		fg = attr & 0x0F;
	if(bg < 0x0)
		bg = ((attr & 0xF0) >> 4);
	attr = EFI_TEXT_ATTR(fg, bg);
	
	wattrset(p->wlist, attr);
	str = p->wlist->text[ui_line];
	mvwprintf(p->wlist, 0, ui_line, str);
	wattroff(p->wlist);

	wrefresh(p->wlist);
}

STATIC VOID set_cwd(struct panel_ctx *p, CONST CHAR16 *path)
{
	if(p->cwd) {
		FreePool(p->cwd);
		p->cwd = NULL;
	}
	if(path)
		StrnCatGrow(&p->cwd, NULL, path, 0);

	mvwhline(p->wcwd, 0, 0, BOXDRAW_HORIZONTAL, p->wcwd->width);
	wattrset(p->wcwd, EFI_TEXT_ATTR(EFI_WHITE, EFI_BLACK));
	mvwprintf(p->wcwd, 0, 0, L" %s ", p->cwd ? p->cwd : L" ");
	wattroff(p->wcwd);
	wrefresh(p->wcwd);
}

STATIC VOID update_file_info(struct panel_ctx *p)
{
	EFI_SHELL_FILE_INFO *file;
	CONST CHAR16 *name;

	if(!p->cwd)
		name = p->fsa->full_name[p->curline - 1];
	else {
		file = dirl_getn(p->dirs, p->curline);
		name = file->FileName;
	}

	mvwprintf(p->wfname, 0, 0, L"%-*s", p->wfname->width, name);
	wrefresh(p->wfname);
}

STATIC VOID update_marked_info(struct panel_ctx *p)
{
	mvwhline(p->wmarked, 0, 0, BOXDRAW_HORIZONTAL, p->wmarked->width);
	if(p->marked) {
		wattrset(p->wmarked, EFI_TEXT_ATTR(EFI_WHITE, EFI_BLACK));
		mvwprintf(p->wmarked, 0, 0, L" Files: %d ", p->marked);
		wattroff(p->wmarked);
	}
	wrefresh(p->wmarked);
}

struct panel_ctx *panel_alloc(struct screen *scr, CONST CHAR16 *path,
		INT32 cols, INT32 lines, INT32 x, INT32 y)
{
	ASSERT(scr != NULL);

	struct panel_ctx *panel;
	UINTN name_cols = cols - 1 - MODIFYTIME_COLS - 1 - SIZE_COLS - 1 - 1;
	BOOLEAN res = FALSE;

	panel = AllocateZeroPool(sizeof(struct panel_ctx));
	if(!panel)
		return NULL;

	do { // START DO

	panel->wbg = newwin(scr, cols, lines, x, y);
	if(!panel->wbg)
		break;
	wborder(panel->wbg,
		BOXDRAW_VERTICAL, BOXDRAW_VERTICAL,
		BOXDRAW_HORIZONTAL, BOXDRAW_HORIZONTAL,
		BOXDRAW_DOWN_RIGHT, BOXDRAW_DOWN_LEFT,
		BOXDRAW_UP_RIGHT, BOXDRAW_UP_LEFT
	);
	mvwprintf(panel->wbg, 1 + ((name_cols - 4) / 2), 1, L"Name");
	mvwvline(panel->wbg, 1 + name_cols, 1, BOXDRAW_VERTICAL, 1);
	mvwprintf(panel->wbg, 1 + name_cols + 1 + ((SIZE_COLS - 4) / 2), 1, L"Size");
	mvwvline(panel->wbg, cols - 1 - MODIFYTIME_COLS - 1, 1, BOXDRAW_VERTICAL, 1);
	mvwprintf(panel->wbg, 1 + name_cols + 1 + SIZE_COLS + 1 + ((MODIFYTIME_COLS - 11) / 2), 1, L"Modify Time");
	
	panel->wcwd = newwin(scr, cols - 4, 1, x + 2, y);
	if(!panel->wcwd)
		break;

	panel->wlist = newwin(scr, cols - 2, lines - 5, x + 1, y + 2);
	if(!panel->wlist)
		break;

	mvwhline(panel->wbg, 1, lines - 3, BOXDRAW_HORIZONTAL, cols - 2);
	panel->wfname = newwin(scr, cols - 2, 1, x + 1, lines - 1);
	if(!panel->wfname)
		break;

	panel->wmarked = newwin(scr, cols - 4, 1, x + 2, lines - 2);
	if(!panel->wmarked)
		break;

	res = TRUE;

	} while(0); // END DO

	if(!res) {
		panel_release(panel);
		return NULL;
	}

	panel->name_cols = name_cols;
	panel->curline = 1;
	panel->list_lines = lines - 5;
	panel->start_entry = 1;

	wrefresh(panel->wbg);
	res = panel_show(panel, path);
	if(!res) {
		panel_release(panel);
		return NULL;
	}
	return panel;
}

VOID panel_release(struct panel_ctx *p)
{
	ASSERT(p != NULL);

	if(p->wbg)
		delwin(p->wbg);
	if(p->wcwd)
		delwin(p->wcwd);
	if(p->wlist)
		delwin(p->wlist);
	if(p->wfname)
		delwin(p->wfname);
	if(p->wmarked)
		delwin(p->wmarked);
	if(p->cwd)
		FreePool(p->cwd);

	FreePool(p);
}
BOOLEAN panel_move_cursor(struct panel_ctx *p, UINTN line)
{
	UINTN oldline = p->curline;
	UINTN maxlen = (p->cwd) ? p->dirs->len : p->fsa->len;

	if(line < 1 || line > maxlen)
		return FALSE;

	if(line < p->start_entry || line >= (p->start_entry + p->list_lines) ||
			p->start_entry == 0)
	{
		p->start_entry = line - ((line-1) % p->list_lines);
		
		if(!p->cwd)
			display_fs(p, p->start_entry);
		else
			display_files(p, p->start_entry);
	}
	else
		UNHIGHLIGHT_LINE_AS_CURRENT(p, oldline);

	p->curline = line;
	update_file_info(p);
	HIGHLIGHT_LINE_AS_CURRENT(p, p->curline);
	return TRUE;
}

BOOLEAN panel_cd_to(struct panel_ctx *p, CONST CHAR16 *path)
{
	struct fs_array *fsa = NULL;
	struct dir_list *dirs = NULL;
	ASSERT(p != NULL);

	if(path) {
		dirs = scandir(path, L"*", 0);
		if(!dirs)
			return FALSE;
	}
	else {
		fsa = scanfs();
		if(!fsa)
			return FALSE;
	}

	set_cwd(p, path);
	if(p->dirs)
		dirl_release(p->dirs);
	if(p->fsa)
		fsa_release(p->fsa);
	p->dirs = dirs;
	p->fsa = fsa;
	p->marked = 0;
	p->start_entry = 0;

	update_marked_info(p);
	panel_move_cursor(p, 1);
	return TRUE;
}

