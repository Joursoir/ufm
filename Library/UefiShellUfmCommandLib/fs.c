#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DevicePathLib.h>
#include <Library/ShellCommandLib.h>

#include "fs.h"

STATIC BOOLEAN search_list(CONST CHAR16 *list, CONST CHAR16 *target,
	OUT CHAR16 **var OPTIONAL, CONST BOOLEAN meta, CONST CHAR16 *delimiter)
{
	CHAR16 *temp_list, *temp_str;
	CONST CHAR16 *list_walker;
	BOOLEAN result;

	for(list_walker = list, temp_list = NULL;
		list_walker != NULL && *list_walker != CHAR_NULL; )
	{
		temp_list = StrnCatGrow(&temp_list, NULL, list_walker, 0);
		ASSERT(temp_list != NULL);

		temp_str = StrStr(temp_list, delimiter);
		if(temp_str != NULL)
			*temp_str = CHAR_NULL;

		list_walker = StrStr(list_walker, delimiter);
		while(list_walker != NULL && *list_walker == *delimiter)
			list_walker++;

		if(meta)
			result = gUnicodeCollation->MetaiMatch(gUnicodeCollation, temp_list, (CHAR16 *)target);
		else
			result = (BOOLEAN)(StrCmp(temp_list, target) == 0);

		if(result) {
			if(var != NULL)
				*var = temp_list;
			else
				FreePool(temp_list);
			return TRUE;
		}
		FreePool(temp_list);
		temp_list = NULL;
	}

	return FALSE;
}

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
		search_list(map_list, L"FS*:", &fsa->full_name[i], TRUE, L";");
		ASSERT(fsa->full_name[i] != NULL);
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
