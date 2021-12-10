#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellLib.h>
#include <Library/PrintLib.h> // UnicodeVSPrint()
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "screen.h"
#include "win.h"

#define CHECK_POSITION(win, x, y) \
	if(x < 0 || y < 0 || x >= win->width || y >= win->height) \
		return FALSE

#define SET_WINDOW_CHAR(win, col, row, character, attrib) \
	win->text[row][col] = character; \
	win->attr[row][col] = attrib

#define SET_WINDOW_CHAR2(win, col, row, character) \
	SET_WINDOW_CHAR(win, col, row, character, win->cur_attr)

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
	win->echo = FALSE;

	for(y = 0; y < nlines; y++) {
		win->text[y] = AllocatePool((ncols + 1) * sizeof(CHAR16));
		win->attr[y] = AllocatePool((ncols + 1) * sizeof(INT32));
		if(!win->text[y] || !win->attr[y]) {
			delwin(win);
			return NULL;
		}

		for(x = 0; x < ncols; x++) {
			SET_WINDOW_CHAR2(win, x, y, L' ');
		}
		SET_WINDOW_CHAR2(win, x, y, CHAR_NULL);
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

VOID echo(struct window *w, BOOLEAN state)
{
	ASSERT(w != NULL);

	w->echo = state;
}

EFI_INPUT_KEY wgetch(struct window *w)
{
	UINTN key_event = 0;
	EFI_SIMPLE_TEXT_INPUT_PROTOCOL *stdin;
	EFI_INPUT_KEY key = { 0 };

	ASSERT(w != NULL);

	stdin = w->scr->stdin;
	gBS->WaitForEvent(1, &stdin->WaitForKey, &key_event);
	stdin->ReadKeyStroke(stdin, &key);

	if(echo) {
		; // print char
	}
	return key;
}

BOOLEAN wmove(struct window *w, INT32 x, INT32 y)
{
	ASSERT(w != NULL);
	CHECK_POSITION(w, x, y);

	w->curx = x;
	w->cury = y;
	return TRUE;
}

BOOLEAN wborder(struct window *w, CHAR16 ls, CHAR16 rs, CHAR16 ts,
	CHAR16 bs, CHAR16 tl, CHAR16 tr, CHAR16 bl, CHAR16 br)
{
	INT32 i, endx, endy;
	ASSERT(w != NULL);

	endx = w->width - 1;
	endy = w->height - 1;

	for(i = 0; i <= endx; i++) {
		SET_WINDOW_CHAR2(w, i, 0, ts);
		SET_WINDOW_CHAR2(w, i, endy, bs);
	}

	for(i = 0; i <= endy; i++) {
		SET_WINDOW_CHAR2(w, 0, i, ls);
		SET_WINDOW_CHAR2(w, endx, i, rs);
	}

	SET_WINDOW_CHAR2(w, 0, 0, tl);
	SET_WINDOW_CHAR2(w, endx, 0, tr);
	SET_WINDOW_CHAR2(w, 0, endy, bl);
	SET_WINDOW_CHAR2(w, endx, endy, br);
	return TRUE;
}

BOOLEAN wvline(struct window *w, INT32 x, INT32 y, CHAR16 ch, INT32 attr, UINTN n)
{
	UINTN i, length;
	ASSERT(w != NULL);
	CHECK_POSITION(w, x, y);

	length = w->height - y;
	if(n != 0 && length > n)
		length = n;

	length += y;
	for(i = y; i < length; i++) {
		if(ch != 0)
			w->text[i][x] = ch;
		if(attr != -1)
			w->attr[i][x] = attr;
	}
	return TRUE;
}

BOOLEAN whline(struct window *w, INT32 x, INT32 y, CHAR16 ch, INT32 attr, UINTN n)
{
	UINTN i, length;
	ASSERT(w != NULL);
	CHECK_POSITION(w, x, y);

	length = w->width - x;
	if(n != 0 && length > n)
		length = n;

	length += x;
	for(i = x; i < length; i++) {
		if(ch != 0)
			w->text[y][i] = ch;
		if(attr != -1)
			w->attr[y][i] = attr;
	}
	return TRUE;
}

VOID waddch(struct window *w, CHAR16 ch, INT32 attr)
{
	INT32 x = w->curx
	INT32 y = w->cury;

	if(x >= w->width) {
		w->curx = 0;
		if(++w->cury >= w->height)
			w->cury = 0;
	}
	else
		w->curx += 1;

	if(ch != 0)
		w->text[y][x] = ch;
	if(attr != -1)
		w->attr[y][x] = attr;
}

BOOLEAN mvwaddch(struct window *w, INT32 x, INT32 y, CHAR16 ch, INT32 attr)
{
	BOOLEAN moved;

	moved = wmove(w, x, y);
	if(moved == FALSE)
		return FALSE;

	waddch(w, ch, attr);
	return TRUE;
}

UINTN EFIAPI wprintf(struct window *w, CONST CHAR16 *fmt, ...)
{
	VA_LIST arg;
	UINTN retval;

	VA_START(arg, fmt);
	retval = wvprintf(w, fmt, arg);
	VA_END(arg);
	return retval;
}

UINTN EFIAPI mvwprintf(struct window *w, INT32 x, INT32 y, CONST CHAR16 *fmt, ...)
{
	VA_LIST arg;
	UINTN retval;
	BOOLEAN moved;

	moved = wmove(w, x, y);
	if(moved == FALSE)
		return 0;

	VA_START(arg, fmt);
	retval = wvprintf(w, fmt, arg);
	VA_END(arg);
	return retval;
}

UINTN EFIAPI wvprintf(struct window *w, CONST CHAR16 *fmt, VA_LIST args)
{
	INT32 x, y;
	UINTN i, length, max_length, walker_size;
	CHAR16 *fmt_walker;

	ASSERT(w != NULL);
	ASSERT(fmt != NULL);

	x = w->curx;
	y = w->cury;

	walker_size = (w->width + 1) * sizeof(CHAR16);
	fmt_walker = AllocateZeroPool(walker_size);
	if(!fmt_walker)
		return 0;

	UnicodeVSPrint(fmt_walker, walker_size, fmt, args);
	length = StrLen(fmt_walker);
	max_length = w->width - x;
	if(length >= max_length) {
		length = max_length;

		w->curx = 0;
		if(++w->cury >= w->height)
			w->cury = 0;
	}
	else
		w->curx += length;

	for(i = 0; i < length; i++) {
		SET_WINDOW_CHAR2(w, (x + i), y, *(fmt_walker + i));
	}
	FreePool(fmt_walker);
	return length;
}

VOID wrefresh(struct window *w)
{
	UINTN x, y;
	INT32 attributes;
	CHAR16 tmp_ch, *print_ptr;
	EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *stdout;

	ASSERT(w != NULL);

	attributes = w->attr[0][0];
	stdout = w->scr->stdout;
	stdout->SetAttribute(stdout, attributes);

	for(y = 0; y < w->height; y++) {
		stdout->SetCursorPosition(stdout, w->begx, w->begy + y);
		print_ptr = w->text[y];

		for(x = 0; x <= (w->width); x++) {
			if(w->text[y][x] == CHAR_NULL || attributes != w->attr[y][x])
			{
				tmp_ch = w->text[y][x];
				w->text[y][x] = CHAR_NULL;
				stdout->OutputString(stdout, print_ptr);
				w->text[y][x] = tmp_ch;

				print_ptr = &(w->text[y][x]);
				if(w->text[y][x] != CHAR_NULL) {
					attributes = w->attr[y][x];
					stdout->SetAttribute(stdout, attributes);
				}
			}
		}
	}

	stdout->SetAttribute(stdout, w->scr->attr);
}
