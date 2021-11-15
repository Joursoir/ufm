#ifndef UFM_MENU_BAR_H
#define UFM_MENU_BAR_H

/*
 * Menu bar: 
 * Located on the first line, occupies its entire length.
 * At the moment, nothing is planned here, a reserve for the future
*/

#include <Uefi.h>

struct screen;

BOOLEAN init_menubar(struct screen *scr);
VOID free_menubar(VOID);

#endif /* UFM_MENU_BAR_H */
