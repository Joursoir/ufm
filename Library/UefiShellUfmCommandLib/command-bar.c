#include <Library/DebugLib.h>

#include "tbi/screen.h"
#include "tbi/win.h"
#include "actions.h"
#include "command-bar.h"

#define BUTTON_LENGTH 2

struct window *init_cmdbar(struct screen *scr, CONST struct shortcut *shortcuts)
{
	UINTN i, cmd_length, attr_button, attr_cmd;
	UINTN amount_cmds = 0;
	struct window *cmdbar;

	attr_button = EFI_TEXT_ATTR(EFI_WHITE, EFI_LIGHTGRAY);
	attr_cmd = EFI_TEXT_ATTR(EFI_DARKGRAY, EFI_CYAN);

	/* Why scr->columns-1 and not src->columns? Because EFI Shell moves
	the cursor to a new line, after writing to the last cell of the line.
	Therefore, menu bar will dissapear from the first line and there will be
	an empty last line */
	cmdbar = newwin(scr, scr->columns - 1, 1, 0, scr->lines - 1);
	if(!cmdbar)
		return NULL;

	for(i = 0; shortcuts[i].type != ACTION_LAST; i++) {
		if(shortcuts[i].type == ACTION_CMD)
			amount_cmds++;
	}

	cmd_length = ((cmdbar->width + 1) - (BUTTON_LENGTH * amount_cmds)) / amount_cmds;
	for(i = 0; shortcuts[i].type != ACTION_LAST; i++) {
		if(shortcuts[i].type == ACTION_CMD) {
			wattrset(cmdbar, attr_button);
			wprintf(cmdbar, L"%*s", BUTTON_LENGTH, shortcuts[i].button_name);
			wattrset(cmdbar, attr_cmd);
			wprintf(cmdbar, L"%-*s", cmd_length, shortcuts[i].cmd_name);
		}
	}

	wattroff(cmdbar);
	cmdbar_refresh(cmdbar);
	return cmdbar;
}

VOID free_cmdbar(struct window *w)
{
	ASSERT(w != NULL);

	delwin(w);
}

VOID cmdbar_refresh(struct window *w)
{
	ASSERT(w != NULL);
	
	wrefresh(w);
}
