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

struct panel_ctx *panel_alloc(struct screen *scr, CONST CHAR16 *path,
		INT32 cols, INT32 lines, INT32 x, INT32 y)
{
	ASSERT(scr != NULL);

	struct panel_ctx *panel;
	UINTN name_cols = cols - 1 - MODIFYTIME_COLS - 1 - SIZE_COLS - 1 - 1;
	UINTN rows = lines - 2;
	BOOLEAN res = FALSE;

	panel = AllocateZeroPool(sizeof(struct panel_ctx));
	if(!panel)
		return NULL;

	do { // START DO

	panel->wname = newwin(scr, name_cols + 2, rows, x, y);
	if(!panel->wname)
		break;
	wborder(panel->wname,
		BOXDRAW_VERTICAL, BOXDRAW_VERTICAL,
		BOXDRAW_HORIZONTAL, BOXDRAW_HORIZONTAL,
		BOXDRAW_DOWN_RIGHT, BOXDRAW_DOWN_HORIZONTAL,
		BOXDRAW_VERTICAL_RIGHT, BOXDRAW_UP_HORIZONTAL
	);
	mvwprintf(panel->wname, 1 + ((name_cols - 4) / 2), 1, L"Name");

	panel->wsize = newwin(scr, SIZE_COLS + 2, rows, x + 1 + name_cols, y);
	if(!panel->wsize)
		break;
	wborder(panel->wsize,
		BOXDRAW_VERTICAL, BOXDRAW_VERTICAL,
		BOXDRAW_HORIZONTAL, BOXDRAW_HORIZONTAL,
		BOXDRAW_DOWN_HORIZONTAL, BOXDRAW_DOWN_HORIZONTAL,
		BOXDRAW_UP_HORIZONTAL, BOXDRAW_UP_HORIZONTAL
	);
	mvwprintf(panel->wsize, 1 + ((SIZE_COLS - 4) / 2), 1, L"Size");

	panel->wmodt = newwin(scr, MODIFYTIME_COLS + 2, rows,
		x + 1 + name_cols + 1 + SIZE_COLS, y);
	if(!panel->wmodt)
		break;
	wborder(panel->wmodt,
		BOXDRAW_VERTICAL, BOXDRAW_VERTICAL,
		BOXDRAW_HORIZONTAL, BOXDRAW_HORIZONTAL,
		BOXDRAW_DOWN_HORIZONTAL, BOXDRAW_DOWN_LEFT,
		BOXDRAW_UP_HORIZONTAL, BOXDRAW_VERTICAL_LEFT
	);
	mvwprintf(panel->wmodt, 1 + ((MODIFYTIME_COLS - 11) / 2), 1, L"Modify Time");

	panel->winfo = newwin(scr, cols, 3, x, y + lines - 3);
	if(!panel->winfo)
		break;
	wborder(panel->winfo,
		BOXDRAW_VERTICAL, BOXDRAW_VERTICAL,
		BOXDRAW_HORIZONTAL, BOXDRAW_HORIZONTAL,
		BOXDRAW_VERTICAL_RIGHT, BOXDRAW_VERTICAL_LEFT,
		BOXDRAW_UP_RIGHT, BOXDRAW_UP_LEFT
	);

	res = TRUE;

	} while(0); // END DO

	panel->cwd = path;
	panel->curline = 1;
	panel->list_lines = rows - 2 - 1;
	panel->start_entry = 1;

	if(!res) {
		panel_release(panel);
		return NULL;
	}

	res = panel_show(panel, panel->cwd);
	if(!res) {
		panel_release(panel);
		return NULL;
	}
	return panel;
}

VOID panel_release(struct panel_ctx *p)
{
	ASSERT(p != NULL);

	if(p->winfo)
		delwin(p->winfo);
	if(p->wname)
		delwin(p->wname);
	if(p->wsize)
		delwin(p->wsize);
	if(p->wmodt)
		delwin(p->wmodt);

	FreePool(p);
}
