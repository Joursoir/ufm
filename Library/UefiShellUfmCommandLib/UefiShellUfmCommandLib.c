#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/ShellCommandLib.h>

#include "tbi/screen.h"
#include "menu-bar.h"
#include "command-bar.h"
#include "panel.h"
#include "actions.h"
#include "filemanager.h"

struct fm_context fm_ctx; // from filemanager.h

STATIC CONST struct shortcut shortcuts[] = {
	{ACTION_ROUTINE, {SCAN_UP,        0x0}, jump_up, L"Up arrow", L"Movement"},
	{ACTION_ROUTINE, {SCAN_DOWN,      0x0}, jump_down, L"Down arrow", L"Movement"},
	{ACTION_ROUTINE, {SCAN_NULL,      CHAR_TAB}, change_panel, L"TAB", L"Movement"},
	{ACTION_ROUTINE, {SCAN_NULL,      CHAR_LINEFEED}, execute, L"Enter", L"Movement"},
	{ACTION_ROUTINE, {SCAN_NULL,      CHAR_CARRIAGE_RETURN}, execute, L"Enter", L"Movement"},
	{ACTION_ROUTINE, {SCAN_NULL,      L' '}, mark, L"Space", L"Movement"},
	{ACTION_CMD,     {SCAN_F1,        0x0}, do_nothing, L"1", L"Help"},
	{ACTION_CMD,     {SCAN_F2,        0x0}, show_filesystems, L"2", L"FSs"},
	{ACTION_CMD,     {SCAN_F3,        0x0}, hexedit, L"3", L"Hex"},
	{ACTION_CMD,     {SCAN_F4,        0x0}, edit, L"4", L"Edit"},
	{ACTION_CMD,     {SCAN_F5,        0x0}, cp, L"5", L"Copy"},
	{ACTION_CMD,     {SCAN_F6,        0x0}, mv, L"6", L"RenMov"},
	{ACTION_CMD,     {SCAN_F7,        0x0}, mkdir, L"7", L"Mkdir"},
	{ACTION_CMD,     {SCAN_F8,        0x0}, rm, L"8", L"Rm"},
	{ACTION_CMD,     {SCAN_F9,        0x0}, do_nothing, L"9", L""},
	{ACTION_CMD,     {SCAN_F10,       0x0}, quit, L"10", L"Quit"},
	{ACTION_LAST,    {SCAN_NULL,      0x0}, NULL, NULL, NULL}
};

/**
 Does endless loop
**/
VOID do_ec()
{
	UINTN i;
	EFI_INPUT_KEY key;
	panel_set_active(fm_ctx.right, FALSE);

	while(fm_ctx.flag_run) {
		key = panel_getch(fm_ctx.curpanel);
		for(i = 0; shortcuts[i].type != ACTION_LAST; i++) {
			if(key.ScanCode == shortcuts[i].key.ScanCode && 
					key.UnicodeChar == shortcuts[i].key.UnicodeChar)
			{	
				shortcuts[i].action();
				break;
			}
		}
	}
}

STATIC BOOLEAN prepare_context()
{
	CONST CHAR16 *cwd;
	UINTN pcols, plines;

	SetMem(&fm_ctx, sizeof(fm_ctx), 0);

	fm_ctx.scr = prepare_screen();
	if(!fm_ctx.scr)
		return FALSE;

	fm_ctx.menubar = init_menubar(fm_ctx.scr);
	if(!fm_ctx.menubar)
		return FALSE;

	fm_ctx.cmdbar = init_cmdbar(fm_ctx.scr, shortcuts);
	if(!fm_ctx.cmdbar)
		return FALSE;

	cwd = ShellGetCurrentDir(NULL);
	pcols = fm_ctx.scr->columns / 2;
	plines = fm_ctx.scr->lines - 2;
	fm_ctx.left = panel_alloc(fm_ctx.scr, cwd, pcols, plines, 0, 1);
	fm_ctx.right = panel_alloc(fm_ctx.scr, cwd, pcols, plines, pcols, 1);

	if(!fm_ctx.left || !fm_ctx.right)
		return FALSE;

	fm_ctx.curpanel = fm_ctx.left;
	fm_ctx.flag_run = TRUE;
	return TRUE;
}

STATIC VOID forget_context()
{
	if(fm_ctx.menubar)
		free_menubar(fm_ctx.menubar);
	if(fm_ctx.cmdbar)
		free_cmdbar(fm_ctx.cmdbar);

	if(fm_ctx.left)
		panel_release(fm_ctx.left);
	if(fm_ctx.right)
		panel_release(fm_ctx.right);

	if(fm_ctx.scr) {
		screen_clear(fm_ctx.scr);
		forget_screen(fm_ctx.scr);
	}
}

SHELL_STATUS EFIAPI ShellCommandRunUFM(
	EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	SHELL_STATUS status = SHELL_LOAD_ERROR;
	BOOLEAN res;

	res = prepare_context();
	if(res) {
		do_ec();
		status = SHELL_SUCCESS;
	}

	forget_context();
	return status;
}

/**
 Return the file name of the help text file if not using HII.
**/
STATIC CONST CHAR16 *EFIAPI ShellCommandGetManFileNameUfm(VOID)
{
	return L"ShellCommand";
}

EFI_STATUS EFIAPI UefiUfmConstructor(
	EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	ShellCommandRegisterCommandName(
		L"ufm",
		ShellCommandRunUFM,
		ShellCommandGetManFileNameUfm,
		0,
		L"ufm",
		TRUE,
		NULL,
		STRING_TOKEN(0)
	);

	return EFI_SUCCESS;
}

EFI_STATUS EFIAPI UefiUfmDestructor(
	EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
	return EFI_SUCCESS;
}
