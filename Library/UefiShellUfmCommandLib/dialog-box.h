#ifndef UFM_DIALOG_BOX_H
#define UFM_DIALOG_BOX_H

#include <Uefi.h>
#include "widget/input.h"

struct dbox_ctx {
	struct window *wbg;
	struct widget_input *in;
	struct window *wok, *wcl;
};

/*
 * Creates a dialog box with given parameters. Appears exactly in the
 * middle of the screen
 *
 * scr: the information of the screen
 * title: the pointer to title string
 * label: the pointer to label string
 * wid_input: TRUE if the dialog box needs an input widget. FALSE if not 
 * input_text: the pointer to initial string in the input widget. If
               wid_input is false, the variable is ignored
 *
 * return: A pointer to the allocated structure or NULL if allocation fails
*/
struct dbox_ctx *dbox_alloc(struct screen *scr, CONST CHAR16 *title,
	CONST CHAR16 *label, BOOLEAN wid_input, CONST CHAR16 *input_text);

/*
 * Deletes the dialog box, frees the structure
 *
 * dbox: the dialog box on which to operate
 *
 * return: VOID
*/
VOID dbox_release(struct dbox_ctx *dbox);

/*
 * Does the output of dialog box to the terminal
 *
 * dbox: the dialog box on which to operate
 *
 * return: VOID
*/
VOID dbox_refresh(struct dbox_ctx *dbox);

#endif /* UFM_DIALOG_BOX_H */
