#include <Uefi.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "screen.h"

struct screen *prepare_screen(VOID)
{
	struct screen *scr;
	scr = AllocateZeroPool(sizeof(struct screen));
	if(!scr)
		return NULL;

	scr->stdin = gST->ConIn;
	scr->stdout = gST->ConOut;
	scr->stderr = gST->StdErr;

	gST->ConOut->QueryMode(
		gST->ConOut,
		gST->ConOut->Mode->Mode,
		&(scr->columns),
		&(scr->lines)
	);
	scr->attr = gST->ConOut->Mode->Attribute;

	return scr;
}

VOID forget_screen(struct screen *scr)
{
	ASSERT(scr != NULL);

	FreePool(scr);
}
