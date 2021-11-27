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
