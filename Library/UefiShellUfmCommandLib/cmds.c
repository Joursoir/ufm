#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/FileHandleLib.h>
#include <Library/MemoryAllocationLib.h>

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
