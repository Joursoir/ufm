#ifndef UFM_PANEL_H
#define UFM_PANEL_H

#include <Uefi.h>

struct screen;
struct window;
struct dir_list;
struct fs_array;

struct panel_ctx {
	BOOLEAN	show_fs; // is filesystems showing now?
	struct window *wbg; // static window
	struct window *wcwd, *wlist, *wfname, *wmarked; // dynamic windows
	UINTN name_cols;

	CONST CHAR16 *cwd; // current work directory

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
 * w: the panel on which to operate
 *
 * return: VOID
*/
VOID panel_release(struct panel_ctx *p);

#endif /* UFM_PANEL_H */
