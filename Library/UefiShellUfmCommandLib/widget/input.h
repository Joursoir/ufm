#ifndef UFM_WIDGET_INPUT_H
#define UFM_WIDGET_INPUT_H

#include <Uefi.h>
#include "tbi/screen.h"
#include "tbi/win.h"

struct widget_input {
	struct window *win;
	INTN point; // cursor position in the input line in chars
	UINTN max_size; // maximum length of input line
	UINTN buf_len; // current length of input line
	CHAR16 *buffer;
};

/*
 * Creates a input widget (single line of text and lets the user edit it) with
 * given parameters.
 *
 * scr: the information of the screen
 * x: the column coordinate (starts from 0) of upper left corner of the window
 * y: the line coordinate (starts from 0) of upper left corner of the window
 * width: the length and the maximum number of characters in the line
 * attr: attributes of the line
 * def_text: the pointer to initial string in the input line
 *
 * return: A pointer to the allocated structure or NULL if allocation fails
*/
struct widget_input *input_alloc(struct screen *scr, INT32 x, INT32 y,
	INT32 width, INT32 attr, CONST CHAR16 *def_text);

/*
 * Deletes the input widget, frees the structure
 *
 * in: the input widget on which to operate
 *
 * return: VOID
*/
VOID input_release(struct widget_input *in);

/*
 * Moves the cursor to the specified position from 0 to current length of
 * the input line. If the position is not in this limit, then extreme 
 * points will be used 
 *
 * in: the input widget on which to operate
 * pos: the coordinate (starts from 0)
 *
 * return: VOID
*/
VOID input_set_point(struct widget_input *in, INTN pos);

/*
 * Handles the given key and print it (if it's character).
 * Supports next control characters: Backspace, left arrow, right arrow.
 * Ignores others.
 *
 * in: the input widget on which to operate
 * key: the key to handle
 *
 * return: VOID
*/
VOID input_handle_char(struct widget_input *in, EFI_INPUT_KEY key);

/*
 * Does the output of input widget to the terminal
 *
 * in: the input widget on which to operate
 *
 * return: VOID
*/
VOID input_update(struct widget_input *in);

#endif /* UFM_WIDGET_INPUT_H */
