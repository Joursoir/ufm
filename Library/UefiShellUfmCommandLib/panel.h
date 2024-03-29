#ifndef UFM_PANEL_H
#define UFM_PANEL_H

#include <Uefi.h>

struct screen;
struct window;
struct dir_list;
struct fs_array;

struct panel_ctx {
	struct window *wbg; // static window
	struct window *wcwd, *wlist, *wfname, *wmarked; // dynamic windows
	UINTN name_cols;

	CHAR16 *cwd; // current work directory
	// if cwd == NULL then we are showing filesystems now

	UINTN curline; // current line
	struct dir_list *dirs; // directory contents
	struct fs_array *fsa; // aviable file systems

	UINTN list_lines; // number of lines in the files list
	UINTN start_entry; // file index at the beginning of the files list
	UINTN marked; // count of marked files
};

/*
 * Creates a panel with given parameters. Filled with files from the
 * specified path
 *
 * scr: the information of the screen
 * path: the pointer to path string
 * cols: the number of columns
 * lines: the number of lines
 * x: the column coordinate (starts from 0) of upper left corner of the window
 * y: the line coordinate (starts from 0) of upper left corner of the window
 *
 * return: A pointer to the allocated structure or NULL if allocation fails
*/
struct panel_ctx *panel_alloc(struct screen *scr, CONST CHAR16 *path,
		INT32 cols, INT32 lines, INT32 x, INT32 y);

/*
 * Deletes the panel, frees the structure
 *
 * p: the panel on which to operate
 *
 * return: VOID
*/
VOID panel_release(struct panel_ctx *p);

/*
 * Moves cursor position to specified line
 *
 * p: the panel on which to operate
 * line: the line coordinate (starts from 1)
 *
 * return: FALSE upon failure and TRUE upon successful completion
*/
BOOLEAN panel_move_cursor(struct panel_ctx *p, UINTN line);

/*
 * Changes the working directory
 *
 * p: the panel on which to operate
 * path: the pointer to path string
 *
 * return: FALSE upon failure and TRUE upon successful completion
*/
BOOLEAN panel_cd_to(struct panel_ctx *p, CONST CHAR16 *path);

/*
 * Changes the active state
 *
 * p: the panel on which to operate
 * active: TRUE if the panel should be activated. FALSE if the
 *         panel should be disabled
 *
 * return: VOID
*/
VOID panel_set_active(struct panel_ctx *p, BOOLEAN active);

/*
 * Marks the file as selected. Supports work only with files.
 *
 * p: the panel on which to operate
 * line: the line coordinate (starts from 1)
 *
 * return: FALSE upon failure and TRUE upon successful completion
*/
BOOLEAN panel_mark_file(struct panel_ctx *p, UINTN line);

/*
 * Gets character from the input device.
 *
 * p: the panel on which to operate
 *
 * return: A pointer to a buffer that is filled in with the keystroke
 *         information for the key that was pressed.
*/
EFI_INPUT_KEY panel_getch(struct panel_ctx *p);

/*
 * Does the output of panel to the terminal
 *
 * w: the panel on which to operate
 *
 * return: VOID
*/
VOID panel_refresh(struct panel_ctx *p);

#endif /* UFM_PANEL_H */
