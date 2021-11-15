#ifndef UFM_TBI_SCREEN_H
#define UFM_TBI_SCREEN_H

/*
	EDK2 NOTE: All devices that support the Simple Text Output Protocol must
	minimally support an 80x25 character mode. Additional modes are optional
*/

#include <Uefi.h>

struct screen
{
	EFI_SIMPLE_TEXT_INPUT_PROTOCOL *stdin;
	EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *stdout;
	EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *stderr;
	UINTN columns, lines; // current screen size
	INT32 attr; // main screen attributes
};

/*
 * Prepares a structure with information about screen
 *
 * return: A pointer to the allocated structure or NULL if allocation fails
*/
struct screen *prepare_screen(VOID);

/*
 * Frees the structure of the screen
 *
 * scr: the screen on which to operate
 *
 * return: VOID
*/
VOID forget_screen(struct screen *scr);

#endif /* UFM_TBI_SCREEN_H */
