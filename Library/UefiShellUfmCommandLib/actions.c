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

STATIC EFI_STATUS shell_exec2(CONST CHAR16 *farg,
	CONST CHAR16 *sarg, EFI_STATUS *cmd_status)
{
	EFI_STATUS status;
	CHAR16 *cmd_line = NULL;
	UINTN size = 0;

	StrnCatGrow(&cmd_line, &size, farg, 0);
	StrnCatGrow(&cmd_line, &size, sarg, 0);
	status = ShellExecute(&gImageHandle, cmd_line, FALSE, NULL, cmd_status);
	SHELL_FREE_NON_NULL(cmd_line);
	return status;
}

STATIC VOID redraw()
{
	panel_refresh(LEFT_PANEL);
	panel_refresh(RIGHT_PANEL);
	menubar_refresh(MENUBAR);
	cmdbar_refresh(CMDBAR);
}

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

BOOLEAN execute(VOID)
{
	EFI_SHELL_FILE_INFO *file;
	EFI_STATUS cmd_status;

	if(!PANEL->cwd)
		return panel_cd_to(PANEL, PANEL->fsa->full_name[PANEL->curline-1]);
	
	file = dirl_getn(PANEL->dirs, PANEL->curline);
	if((file->Info->Attribute & EFI_FILE_DIRECTORY) != 0)
		return panel_cd_to(PANEL, file->FullName);

	shell_exec2(L"", file->FullName, &cmd_status);
	redraw();
	return TRUE;
}

BOOLEAN mark(VOID)
{
	panel_mark_file(PANEL, PANEL->curline);
	return TRUE;
}

BOOLEAN edit(VOID)
{
	EFI_SHELL_FILE_INFO *file;
	EFI_STATUS cmd_status;

	if(!PANEL->cwd)
		return FALSE;

	file = dirl_getn(PANEL->dirs, PANEL->curline);
	if((file->Info->Attribute & EFI_FILE_DIRECTORY) != 0)
		return FALSE;

	shell_exec2(L"edit ", file->FullName, &cmd_status);
	redraw();
	return TRUE;
}

BOOLEAN hexedit(VOID)
{
	EFI_SHELL_FILE_INFO *file;
	EFI_STATUS cmd_status;

	if(!PANEL->cwd)
		return FALSE;

	file = dirl_getn(PANEL->dirs, PANEL->curline);
	if((file->Info->Attribute & EFI_FILE_DIRECTORY) != 0)
		return FALSE;

	shell_exec2(L"hexedit ", file->FullName, &cmd_status);
	redraw();
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
