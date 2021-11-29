#include <Library/DebugLib.h>

#include "tbi/screen.h"
#include "tbi/win.h"
#include "menu-bar.h"

struct window *init_menubar(struct screen *scr)
{
	struct window *menubar = newwin(scr, scr->columns, 1, 0, 0);
	if(!menubar)
		return NULL;

	mvwprintf(menubar, 0, 0, L"Menu bar");
	menubar_refresh(menubar);
	return menubar;
}

VOID free_menubar(struct window *w)
{
	ASSERT(w != NULL);

	delwin(w);
}

VOID menubar_refresh(struct window *w)
{
	ASSERT(w != NULL);
	
	wrefresh(w);
}
