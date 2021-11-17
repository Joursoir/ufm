#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DevicePathLib.h>
#include <Library/ShellCommandLib.h>

#include "fs.h"

struct fs_array *fsa_alloc(EFI_HANDLE *handles, UINTN count)
{
	UINTN i;
	struct fs_array *fsa;
	EFI_DEVICE_PATH_PROTOCOL *dev_path;
	CONST CHAR16 *map_list;

	fsa = AllocatePool(sizeof(struct fs_array));
	if(!fsa)
		return NULL;

	fsa->full_name = AllocateZeroPool(count * sizeof(CHAR16 *));
	if(!fsa->full_name) {
		fsa_release(fsa);
		return NULL;
	}

	fsa->len = count;
	for(i = 0; i < count; i++) {
		dev_path = DevicePathFromHandle(handles[i]);
		map_list = gEfiShellProtocol->GetMapFromDevicePath(&dev_path);
	}

	return fsa;
}

VOID fsa_release(struct fs_array *fsa)
{
	UINTN i;
	if(fsa->full_name) {
		for(i = 0; i < fsa->len; i++) {
			if(fsa->full_name[i]) // safety measure and our good sleep
				FreePool(fsa->full_name[i]);
		}
		FreePool(fsa->full_name);
	}
	FreePool(fsa);
}
