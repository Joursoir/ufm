#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/ShellLib.h>
#include <Library/BaseLib.h>
#include <Library/MemoryAllocationLib.h>

#include "dir.h"

struct dir_list *dirl_alloc(CHAR16 *search_path, CONST UINT64 attr)
{
	EFI_STATUS status = EFI_SUCCESS;
	EFI_SHELL_FILE_INFO *list_head = NULL, *node = NULL;
	UINTN length = 0;
	struct dir_list *dl;

	status = ShellOpenFileMetaArg(search_path, EFI_FILE_MODE_READ, &list_head);
	if(EFI_ERROR(status))
		return NULL;

	for(node = (EFI_SHELL_FILE_INFO *)GetFirstNode(&list_head->Link);
			!IsNull(&list_head->Link, &node->Link);
			node = (EFI_SHELL_FILE_INFO *)GetNextNode(&list_head->Link, &node->Link))
	{
		ASSERT(node != NULL);
		ASSERT(node->Info != NULL);
		ASSERT((node->Info->Attribute & EFI_FILE_VALID_ATTR) == node->Info->Attribute);

		if((node->Info->Attribute & attr) != attr ||
				StrCmp(node->FileName, L".") == 0) {
			EFI_SHELL_FILE_INFO *tmp;
			tmp = (EFI_SHELL_FILE_INFO *)GetPreviousNode(&list_head->Link, &node->Link);
			
			// I have stolen code below in the source code of InternalFreeShellFileInfoNode():
			RemoveEntryList(&node->Link);
			FreePool((VOID*)node->Info);
			FreePool((VOID*)node->FileName);
			FreePool((VOID*)node->FullName);
			gEfiShellProtocol->CloseFile(node->Handle);
			FreePool(node);
			node = tmp;
			continue;
		}

		length++;
	}

	dl = AllocatePool(sizeof(struct dir_list));
	if(!dl) {
		ShellCloseFileMetaArg(&list_head);
		return NULL;
	}

	dl->list_head = list_head;
	dl->len = length;
	return dl;
}	

VOID dirl_release(struct dir_list *dl)
{
	if(dl->list_head)
		ShellCloseFileMetaArg(&dl->list_head);

	FreePool(dl);
}

struct dir_list *scandir(CONST CHAR16 *search_path, CONST CHAR16 *wildcard,
	CONST UINT64 attr)
{
	UINTN i, path_size = 0;
	CHAR16 *path = NULL;
	struct dir_list *list;

	path = StrnCatGrow(&path, &path_size, search_path, 0);
	if(!path)
		return NULL;

	i = StrLen(path) - 1;
	if(path[i] != L'\\' && path[i] != L'/')
		path = StrnCatGrow(&path, &path_size, L"\\", 0);

	path = StrnCatGrow(&path, &path_size, wildcard, 0);
	if(!path)
		return NULL;

	PathCleanUpDirectories(path);
	list = dirl_alloc(path, attr);

	SHELL_FREE_NON_NULL(path);
	return list;
}
