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

BOOLEAN jump_up(VOID)
{
	panel_move_cursor(fm_ctx.curpanel, fm_ctx.curpanel->curline - 1);
	return TRUE;
}

BOOLEAN jump_down(VOID)
{
	panel_move_cursor(fm_ctx.curpanel, fm_ctx.curpanel->curline + 1);
	return TRUE;
}

BOOLEAN change_panel(VOID)
{
	panel_set_active(fm_ctx.curpanel, FALSE);
	fm_ctx.curpanel = (fm_ctx.curpanel == fm_ctx.left) ? fm_ctx.right : fm_ctx.left;
	panel_set_active(fm_ctx.curpanel, TRUE);
	return TRUE;
}

BOOLEAN mark(VOID)
{
	panel_mark_file(fm_ctx.curpanel, fm_ctx.curpanel->curline);
	return TRUE;
}

BOOLEAN show_filesystems(VOID)
{
	return panel_cd_to(fm_ctx.curpanel, NULL);
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
