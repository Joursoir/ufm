#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellLib.h>
#include <Library/MemoryAllocationLib.h>

#include "screen.h"
#include "win.h"

struct window *newwin(struct screen *s, 
	INT32 ncols, INT32 nlines, INT32 begin_x, INT32 begin_y)
{
	INTN x, y;
	struct window *win;

	ASSERT(s != NULL);

	if(ncols <= 0 || nlines <= 0 || begin_x < 0 || begin_y < 0)
		return NULL;

	if((begin_x + ncols) > s->columns || (begin_y + nlines) > s->lines)
		return NULL;

	win = AllocateZeroPool(sizeof(struct window));
	if(!win)
		return NULL;

	win->text = AllocatePool(nlines * sizeof(CHAR16 *));
	if(!win->text) {
		delwin(win);
		return NULL;
	}

	win->attr = AllocatePool(nlines * sizeof(INT32 *));
	if(!win->attr) {
		delwin(win);
		return NULL;
	}

	win->scr = s;
	win->curx = 0;
	win->cury = 0;
	win->begx = begin_x;
	win->begy = begin_y;
	win->width = ncols;
	win->height = nlines;
	win->cur_attr = s->attr;

	for(y = 0; y < nlines; y++) {
		win->text[y] = AllocatePool((ncols + 1) * sizeof(CHAR16));
		win->attr[y] = AllocatePool((ncols + 1) * sizeof(INT32));
		if(!win->text[y] || !win->attr[y]) {
			delwin(win);
			return NULL;
		}

		for(x = 0; x < ncols; x++) {
			win->text[y][x] = L' ';
			win->attr[y][x] = win->cur_attr;
		}
		win->text[y][x] = CHAR_NULL;
		win->attr[y][x] = win->cur_attr;
	}

	return win;
}

VOID delwin(struct window *w)
{
	INTN i;
	ASSERT(w != NULL);

	if(w->text) {
		for(i = 0; i < w->height; i++) {
			if(w->text[i])
				FreePool(w->text[i]);
		}
		FreePool(w->text);
	}

	if(w->attr) {
		for(i = 0; i < w->height; i++) {
			if(w->attr[i])
				FreePool(w->attr[i]);
		}
		FreePool(w->attr);
	}

	FreePool(w);
}

VOID wattrset(struct window *w, INT32 attr)
{
	ASSERT(w != NULL);

	w->cur_attr = attr;
}

VOID wattroff(struct window *w)
{
	ASSERT(w != NULL);

	w->cur_attr = w->scr->attr;
}
