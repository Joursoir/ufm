#include <Library/UefiLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>

#include "cmds.h"

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
