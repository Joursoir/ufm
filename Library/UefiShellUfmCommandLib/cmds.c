#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/FileHandleLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/ShellCommandLib.h>
#include <Library/SortLib.h>

#include "cmds.h"

STATIC BOOLEAN is_dir_empty(SHELL_FILE_HANDLE FileHandle)
{
	EFI_STATUS status;
	EFI_FILE_INFO *file_info = NULL;
	BOOLEAN no_file = FALSE;
	BOOLEAN ret = TRUE;

	for(status = FileHandleFindFirstFile(FileHandle, &file_info);
		!no_file && !EFI_ERROR (status);
		FileHandleFindNextFile(FileHandle, file_info, &no_file))
	{
		if(StrStr(file_info->FileName, L".") != file_info->FileName &&
				StrStr(file_info->FileName, L"..") != file_info->FileName) {
			ret = FALSE;
		}
	}
	return ret;
}

/**
  String comparison without regard to case for a limited number of characters.

  Analog of StrniCmp in UefiShellLevel2CommandsLib
**/
STATIC INTN StrNoCaseCmpN(CONST CHAR16 *src, CONST CHAR16 *target, CONST UINTN cnt)
{
	CHAR16 *src_copy, *target_copy;
	UINTN src_length, target_length;
	INTN res;

	if(cnt == 0)
		return 0;

	src_length = StrLen(src);
	target_length = StrLen(target);
	src_length = MIN(src_length, cnt);
	target_length = MIN(target_length, cnt);
	src_copy = AllocateCopyPool((src_length + 1) * sizeof (CHAR16), src);
	if(!src_copy)
		return -1;

	target_copy = AllocateCopyPool((target_length + 1) * sizeof (CHAR16), target);
	if(!target_copy) {
		FreePool (src_copy);
		return -1;
	}

	src_copy[src_length] = L'\0';
	target_copy[target_length] = L'\0';
	res = gUnicodeCollation->StriColl(gUnicodeCollation, src_copy, target_copy);
	FreePool(src_copy);
	FreePool(target_copy);
	return res;
}

EFI_STATUS delete_file(EFI_SHELL_FILE_INFO *node)
{
	EFI_SHELL_FILE_INFO *list = NULL, *walker = NULL;
	EFI_STATUS status = EFI_SUCCESS;

	if((node->Info->Attribute & EFI_FILE_READ_ONLY) == EFI_FILE_READ_ONLY)
		return EFI_ACCESS_DENIED;

	if((node->Info->Attribute & EFI_FILE_DIRECTORY) == EFI_FILE_DIRECTORY) {
		if(!is_dir_empty(node->Handle))
		{
			status = gEfiShellProtocol->FindFilesInDir(node->Handle, &list);
			if(EFI_ERROR(status)) {
				if(list)
					gEfiShellProtocol->FreeFileList(&list);
				return EFI_DEVICE_ERROR;
			}

			for(walker = (EFI_SHELL_FILE_INFO *)GetFirstNode(&list->Link);
				!IsNull(&list->Link, &walker->Link);
				walker = (EFI_SHELL_FILE_INFO *)GetNextNode(&list->Link, &walker->Link))
			{
				if(StrCmp(walker->FileName, L".") == 0 || StrCmp(walker->FileName, L"..") == 0)
					continue;

				walker->Status = gEfiShellProtocol->OpenFileByName(walker->FullName, &walker->Handle, EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE);
				if(!EFI_ERROR(walker->Status))
					status = delete_file(walker);
				else
					status = walker->Status;

				if(status != EFI_SUCCESS) {
					if(list)
						gEfiShellProtocol->FreeFileList(&list);
					return status;
				}
			}
			if(list)
				gEfiShellProtocol->FreeFileList(&list);
		}
	}

	if(StrCmp(node->FileName, L".") != 0 && 
			StrCmp(node->FileName, L"..") != 0) {
		// delete the current node
		status = gEfiShellProtocol->DeleteFile(node->Handle);
		node->Handle = NULL;
	}

	return status;
}

STATIC EFI_STATUS copy_to_dir(CONST EFI_SHELL_FILE_INFO *list,
	CONST CHAR16 *dest)
{
	CONST EFI_SHELL_FILE_INFO *node;
	CONST CHAR16 *cwd = ShellGetCurrentDir(NULL);
	CHAR16 *dest_path = NULL;
	UINTN new_length = 0, max_length = 0;
	EFI_STATUS status;

	ASSERT(list != NULL);
	ASSERT(dest != NULL);

	if(dest[0] == L'\\' || StrStr(dest, L":") == NULL) {
		if(!cwd)
			return EFI_INVALID_PARAMETER;
	}

	status = ShellIsDirectory(dest);
	if(EFI_ERROR(status))
		return EFI_INVALID_PARAMETER;

	for(node = (EFI_SHELL_FILE_INFO *)GetFirstNode(&list->Link); 
		!IsNull(&list->Link, &node->Link);
		node = (EFI_SHELL_FILE_INFO *)GetNextNode(&list->Link, &node->Link))
	{
		if(StrCmp(node->FileName, L".") == 0 || StrCmp(node->FileName, L"..") == 0)
			continue;

		new_length = StrLen(dest) + StrLen(node->FullName) + 1;
		new_length += (cwd == NULL) ? 0 : (StrLen(cwd) + 1);
		if(new_length > max_length)
			max_length = new_length;
	}

	dest_path = AllocateZeroPool(max_length * sizeof(CHAR16));
	if(!dest_path)
		return EFI_OUT_OF_RESOURCES;

	for(node = (EFI_SHELL_FILE_INFO *)GetFirstNode(&list->Link);
		!IsNull(&list->Link, &node->Link);
		node = (EFI_SHELL_FILE_INFO *)GetNextNode(&list->Link, &node->Link))
	{
		if(StrCmp(node->FileName, L".") == 0 || StrCmp(node->FileName, L"..") == 0)
			continue;

		if(dest[0] == L'\\') {
			StrCpyS(dest_path, max_length, cwd);
			StrCatS(dest_path, max_length, L"\\");
			while(PathRemoveLastItem(dest_path));
			StrCatS(dest_path, max_length, dest+1);
		}
		else if(StrStr(dest, L":") == NULL) {
			StrCpyS(dest_path, max_length, cwd);
			StrCatS(dest_path, max_length, L"\\");

			if(dest_path[StrLen(dest_path) - 1] != L'\\' && dest[0] != L'\\')
				StrCatS(dest_path, max_length, L"\\");
			else if(dest_path[StrLen(dest_path) - 1] == L'\\' && dest[0] == L'\\')
				((CHAR16*)dest_path)[StrLen(dest_path) - 1] = CHAR_NULL;

			StrCatS(dest_path, max_length, dest);

			if(dest[StrLen(dest) - 1] != L'\\' && node->FileName[0] != L'\\')
				StrCatS(dest_path, max_length, L"\\");
			else if(dest[StrLen(dest) - 1] == L'\\' && node->FileName[0] == L'\\')
				((CHAR16*)dest_path)[StrLen(dest_path) - 1] = CHAR_NULL;
		}
		else {
			StrCpyS(dest_path, max_length, dest);

			if(dest[StrLen(dest) - 1] != L'\\' && node->FileName[0] != L'\\')
				StrCatS(dest_path, max_length, L"\\");
			else if(dest[StrLen(dest) - 1] == L'\\' && node->FileName[0] == L'\\')
				((CHAR16*)dest)[StrLen(dest) - 1] = CHAR_NULL;
		}
		StrCatS(dest_path, max_length, node->FileName);

		if(!EFI_ERROR(ShellIsDirectory(node->FullName)) &&
			!EFI_ERROR(ShellIsDirectory(dest_path)) &&
			StrNoCaseCmpN(node->FullName, dest_path, StrLen(dest_path)) == 0)
		{
			// parent dir
			status = EFI_INVALID_PARAMETER;
			break;
		}

		if(StringNoCaseCompare(&node->FullName, &dest_path) == 0) {
			// same dir
			status = EFI_INVALID_PARAMETER;
			break;
		}

		if((StrNoCaseCmpN(node->FullName, dest_path, StrLen(node->FullName)) == 0) &&
			(dest_path[StrLen(node->FullName)] == CHAR_NULL || 
			dest_path[StrLen(node->FullName)] == L'\\'))
		{
			// same dir
			status = EFI_INVALID_PARAMETER;
			break;
		}

		PathCleanUpDirectories(dest_path);
		status = copy_file(node->FullName, dest_path);
		if(status != EFI_SUCCESS)
			break;
	}

	SHELL_FREE_NON_NULL(dest_path);
	return status;
}

STATIC EFI_STATUS copy_single_file(CONST CHAR16 *src, CONST CHAR16 *dest)
{
	SHELL_FILE_HANDLE src_handle = NULL, dest_handle = NULL;
	UINTN read_size = PcdGet32(PcdShellFileOperationSize);
	VOID *buffer;
	EFI_STATUS status;

	ShellDeleteFileByName(dest);

	status = ShellOpenFileByName(dest, &dest_handle, EFI_FILE_MODE_READ|EFI_FILE_MODE_WRITE|EFI_FILE_MODE_CREATE, 0);
	if(EFI_ERROR(status)) 
		return EFI_ACCESS_DENIED;

	status = ShellOpenFileByName(src, &src_handle, EFI_FILE_MODE_READ, 0);
	if(EFI_ERROR(status)) 
		return EFI_ACCESS_DENIED;

	buffer = AllocateZeroPool(read_size);
	if(buffer == NULL)
		return EFI_OUT_OF_RESOURCES;

	while(read_size == PcdGet32(PcdShellFileOperationSize) && !EFI_ERROR(status))
	{
		status = ShellReadFile(src_handle, &read_size, buffer);
		if(EFI_ERROR(status))
			break;

		status = ShellWriteFile(dest_handle, &read_size, buffer);
		if(EFI_ERROR(status))
			break;
	}

	if(dest_handle != NULL)
		ShellCloseFile(&dest_handle);
	if(src_handle != NULL)
		ShellCloseFile(&src_handle);
	return status;
}

EFI_STATUS copy_file(CONST CHAR16 *src, CONST CHAR16 *dest)
{
	SHELL_FILE_HANDLE dest_handle = NULL;
	EFI_SHELL_FILE_INFO *list = NULL;
	EFI_STATUS status;
	CONST CHAR16 *path_last_item;
	CHAR16 *temp_name = NULL, *correct_dest = NULL;
	UINTN size = 0;

	if(StrCmp(src, dest) == 0)
		return EFI_SUCCESS;

	if(ShellIsDirectory(src) == EFI_SUCCESS) {
		// move DIRECTORY to DIRECTORY
		status = ShellCreateDirectory(dest, &dest_handle);
		if(EFI_ERROR(status))
			return EFI_ACCESS_DENIED;

		StrnCatGrow(&temp_name, &size, src, 0);
		StrnCatGrow(&temp_name, &size, L"\\*", 0);
		if(temp_name == NULL)
			return EFI_OUT_OF_RESOURCES;

		status = ShellOpenFileMetaArg(temp_name, EFI_FILE_MODE_READ, &list);
		if(!EFI_ERROR(status)) {
			*temp_name = CHAR_NULL;
			StrnCatGrow(&temp_name, &size, dest, 0);
			StrnCatGrow(&temp_name, &size, L"\\", 0);
			status = copy_to_dir(list, temp_name);
			ShellCloseFileMetaArg(&list);
		}
		SHELL_FREE_NON_NULL(temp_name);
		return status;
	}

	StrnCatGrow(&correct_dest, &size, dest, 0);
	if(ShellIsDirectory(dest) == EFI_SUCCESS) {
		// move SOURCE to DIRECTORY
		path_last_item = src;
		while((temp_name = StrStr(path_last_item, L"\\")))
			path_last_item = temp_name + 1;
		ASSERT(path_last_item != NULL);

		// dest would be "dest/(src last item)"
		StrnCatGrow(&correct_dest, &size, L"\\", 0);
		StrnCatGrow(&correct_dest, &size, path_last_item, 0);
	}

	status = copy_single_file(src, correct_dest);
	SHELL_FREE_NON_NULL(correct_dest);
	return status;
}

EFI_STATUS make_directory(CONST CHAR16 *dir_name)
{
	EFI_STATUS status = EFI_SUCCESS;
	CHAR16 *tmp_str = NULL;
	CHAR16 split_char, *split_name = NULL;
	SHELL_FILE_HANDLE file_handle = NULL;

	// check if dir already exists
	status = ShellOpenFileByName(dir_name,
		&file_handle,
		EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE,
		EFI_FILE_DIRECTORY
	);
	if(!EFI_ERROR(status)) {
		ShellCloseFile(&file_handle);
		return EFI_INVALID_PARAMETER;
	}

	// create the nested directory from parent to child:
	// if dir_name = test1\test2\test3, first create "test1\" directory, then "test1\test2\", finally "test1\test2\test3".
	tmp_str = AllocateCopyPool(StrSize(dir_name), dir_name);
	tmp_str = PathCleanUpDirectories(tmp_str);
	if(tmp_str == NULL)
		return EFI_OUT_OF_RESOURCES;

	split_name = tmp_str;
	while(split_name != NULL) {
		split_name = StrStr(split_name + 1, L"\\");
		if(split_name != NULL) {
			split_char = *(split_name + 1);
			*(split_name + 1) = '\0';
		}

		// check if current nested directory already exists
		status = ShellOpenFileByName(tmp_str,
			&file_handle,
			EFI_FILE_MODE_READ,
			EFI_FILE_DIRECTORY
		);
		if(!EFI_ERROR(status)) {
			ShellCloseFile(&file_handle);
		}
		else {
			status = ShellCreateDirectory(tmp_str, &file_handle);
			if(EFI_ERROR(status))
				break;
			if(file_handle != NULL)
				gEfiShellProtocol->CloseFile(file_handle);
		}

		if(split_name != NULL)
			*(split_name + 1) = split_char;
	}
	if(EFI_ERROR(status))
		status = EFI_ACCESS_DENIED;
	
	SHELL_FREE_NON_NULL(tmp_str);
	return status;
}
