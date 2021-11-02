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

/*
 * Sets the current attributes of the given window
 *
 * w: the window on which to operate
 * attr: the attributes
 *
 * return: VOID
*/
VOID wattrset(struct window *w, INT32 attr);

/*
 * Resets the current attributes of the given window to standard screen
 * attributes
 *
 * w: the window on which to operate
 *
 * return: VOID
*/
VOID wattroff(struct window *w);

/*
 * Moves cursor position to specified coordinates
 *
 * w: the window on which to operate
 * x: the X(column) coordinate
 * y: the Y(row) coordinate
 *
 * return: FALSE upon failure and TRUE upon successful completion
*/
BOOLEAN wmove(struct window *w, INT32 x, INT32 y);

/*
 * Draws a box around the edges of a window
 *
 * w: the window on which to operate
 * ls: the left side character
 * rs: the right side character
 * ts: the top side character
 * bs: the bottom side character
 * tl: the top left corner character
 * tr: the top right corner character
 * bl: the bottom left corner character
 * br: the bottom right corner character
 *
 * return: FALSE upon failure and TRUE upon successful completion
*/
BOOLEAN wborder(struct window *w, CHAR16 ls, CHAR16 rs, CHAR16 ts,
	CHAR16 bs, CHAR16 tl, CHAR16 tr, CHAR16 bl, CHAR16 br);

/*
 * Moves to specified coordinates, draws a horizontal line using ch
 * starting at (x, y) in the window. The current cursor position is 
 * not changed.
 *
 * w: the window on which to operate
 * x: the X(column) coordinate for the start of the line
 * y: the Y(row) coordinate for the start of the line
 * ch: the character used to draw the line
 * n: the maximum number of chars in the line
 *
 * return: FALSE upon failure and TRUE upon successful completion
*/
BOOLEAN mvwhline(struct window *w, INT32 x, INT32 y, CHAR16 ch, INT32 n);

/*
 * Moves to specified coordinates, draws a vertical line using ch
 * starting at (x, y) in the window. The current cursor position is 
 * not changed.
 *
 * w: the window on which to operate
 * x: the X(column) coordinate for the start of the line
 * y: the Y(row) coordinate for the start of the line
 * ch: the character used to draw the line
 * n: the maximum number of chars in the line
 *
 * return: FALSE upon failure and TRUE upon successful completion
*/
BOOLEAN mvwvline(struct window *w, INT32 x, INT32 y, CHAR16 ch, INT32 n);

/*
 * Prints formatted output on the cursor coordinates
 *
 * w: the window on which to operate
 * fmt: a null-terminated unicode format string
 * ...: variable arguments list whose contents are accessed based on the
        fmt string specified by FormatString
 *
 * return: the number of unicode chars in the produced output buffer
           not including the null-terminator
*/
UINTN EFIAPI wprintf(struct window *w, CONST CHAR16 *fmt, ...);

/*
 * Moves to specified coordinates, prints formatted output
 *
 * w: the window on which to operate
 * x: the X(column) coordinate to move
 * y: the Y(row) coordinate to move
 * fmt: a null-terminated unicode format string
 * ...: variable arguments list whose contents are accessed based on the
        fmt string specified by FormatString
 *
 * return: the number of unicode chars in the produced output buffer
           not including the null-terminator
*/
UINTN EFIAPI mvwprintf(struct window *w, INT32 x, INT32 y, CONST CHAR16 *fmt, ...);

/*
 * Prints output based on a null-terminated unicode format string
   and a arguments list
 *
 * w: the window on which to operate
 * fmt: a null-terminated unicode format string
 * arg: the variable argument list
 *
 * return: the number of unicode chars in the produced output buffer
           not including the null-terminator
*/
UINTN EFIAPI wvprintf(struct window *w, CONST CHAR16 *fmt, VA_LIST args);

#endif /* UFM_TBI_WINDOW_H */
