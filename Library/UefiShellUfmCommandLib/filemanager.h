#ifndef UFM_FILE_MANAGER_H
#define UFM_FILE_MANAGER_H

#include <Uefi.h>

struct screen;
struct window;
struct panel_ctx;

struct fm_context {
	struct screen *scr;
	struct window *menubar, *cmdbar;

	struct panel_ctx *left, *right;
	struct panel_ctx *curpanel;

	BOOLEAN flag_run;
};

extern struct fm_context fm_ctx;

#endif /* UFM_FILE_MANAGER_H */
