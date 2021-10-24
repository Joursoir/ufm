#ifndef UFM_TBI_WINDOW_H
#define UFM_TBI_WINDOW_H

/*
	Window - rectangular area of the screen with which you can work as a separate
	screen (display text, clear, etc). The output should be carried out only in
	the specified rectangular area of the screen

	NOTE: SimpleTextOut.h has defined box-drawing character ("BOXDRAW_*", for
	example BOXDRAW_VERTICAL)
*/

#include <Uefi.h>

struct screen;

struct window {
	struct screen *scr; // parent screen

	INT32 curx, cury; // current cursor position
	INT32 begx, begy; // screen coords of upper left corner
	INT32 width, height; // window size

	CHAR16 **text; // the actual text of whole screen
	/* ATTRIBUTES:
	Bits 0..3 are the foreground color.
	Bits 4..6 are the background color.
	ALl other bits are undefined and must be zero */
	INT32 **attr;

	INT32 cur_attr;
};

/*
 * Creates a window with given parameters
 *
 * s: the information of the screen
 * ncols: the number of columns
 * nlines: the number of lines
 * begin_x: the column coordinate (starts from 0) of upper left corner of the window
 * begin_y: the line coordinate (starts from 0) of upper left corner of the window
 *
 * return: A pointer to the allocated structure or NULL if allocation fails
*/
struct window *newwin(struct screen *s,
	INT32 ncols, INT32 nlines, INT32 begin_x, INT32 begin_y);

/*
 * Deletes the window, frees the structure
 *
 * w: the window on which to operate
 *
 * return: VOID
*/
VOID delwin(struct window *w);

#endif /* UFM_TBI_WINDOW_H */
