#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "dir.h"
#include "fs.h"

#include "menu-bar.h"
#include "command-bar.h"
#include "panel.h"
#include "filemanager.h"
#include "actions.h"

#define MENUBAR (fm_ctx.menubar)
#define CMDBAR (fm_ctx.cmdbar)
#define PANEL (fm_ctx.curpanel)
#define LEFT_PANEL (fm_ctx.left)
#define RIGHT_PANEL (fm_ctx.right)

BOOLEAN jump_up(VOID)
{
	panel_move_cursor(PANEL, PANEL->curline - 1);
	return TRUE;
}

BOOLEAN jump_down(VOID)
{
	panel_move_cursor(PANEL, PANEL->curline + 1);
	return TRUE;
}

BOOLEAN change_panel(VOID)
{
	panel_set_active(PANEL, FALSE);
	PANEL = (PANEL == LEFT_PANEL) ? RIGHT_PANEL : LEFT_PANEL;
	panel_set_active(PANEL, TRUE);
	return TRUE;
}

BOOLEAN mark(VOID)
{
	panel_mark_file(PANEL, PANEL->curline);
	return TRUE;
}

BOOLEAN show_filesystems(VOID)
{
	return panel_cd_to(PANEL, NULL);
}

BOOLEAN do_nothing(VOID)
{
	return TRUE;
}

BOOLEAN quit(VOID)
{
	fm_ctx.flag_run = FALSE;
	return TRUE;
}
