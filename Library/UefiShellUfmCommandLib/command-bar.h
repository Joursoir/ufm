#ifndef UFM_COMMAND_BAR_H
#define UFM_COMMAND_BAR_H

/*
 * Command bar: 
 * Located on the last line, occupies its entire length.
 * Contains a list of actions and buttons that require to press for
 * perform the operation.
*/

#include <Uefi.h>

struct screen;
struct window;
struct shortcut;

struct window *init_cmdbar(struct screen *scr, CONST struct shortcut *shortcuts);
VOID free_cmdbar(struct window *w);
VOID cmdbar_refresh(struct window *w);

#endif /* UFM_COMMAND_BAR_H */
