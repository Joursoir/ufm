#include <Library/DebugLib.h>

#include "tbi/screen.h"
#include "tbi/win.h"
#include "menu-bar.h"

struct window *menubar = NULL;

BOOLEAN init_menubar(struct screen *scr)
{
	menubar = newwin(scr, scr->columns, 1, 0, 0);
	if(!menubar)
		return FALSE;

	wrefresh(menubar);
	return TRUE;
}

VOID free_menubar(VOID)
{
	ASSERT(menubar != NULL);

	delwin(menubar);
	menubar = NULL;
}
