#ifndef UFM_ACTIONS_H
#define UFM_ACTIONS_H

#include <Uefi.h>

enum ACTION_TYPE {
	ACTION_ROUTINE = 0,
	ACTION_CMD,
	ACTION_LAST
};

struct shortcut {
	enum ACTION_TYPE type;
	EFI_INPUT_KEY key;
	BOOLEAN (*action)(VOID);
	CHAR16 *button_name;
	CHAR16 *cmd_name;
};

BOOLEAN jump_up(VOID);
BOOLEAN jump_down(VOID);
BOOLEAN change_panel(VOID);
BOOLEAN execute(VOID);
BOOLEAN mark(VOID);
BOOLEAN edit(VOID);
BOOLEAN hexedit(VOID);
BOOLEAN mkdir(VOID);
BOOLEAN show_filesystems(VOID);
BOOLEAN do_nothing(VOID);
BOOLEAN quit(VOID);

#endif /* UFM_ACTIONS_H */
