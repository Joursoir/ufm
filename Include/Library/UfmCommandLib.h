#ifndef UFM_COMMAND_LIB_H_
#define UFM_COMMAND_LIB_H_

/**
  UEFI File Manager.

  @param[in] ImageHandle  Handle to the Image (NULL if internal).
  @param[in] SystemTable  Pointer to the System Table (NULL if internal).

  @retval SHELL_INVALID_PARAMETER The command line invocation could not be parsed.
  @retval SHELL_NOT_FOUND         The command failed.
  @retval SHELL_SUCCESS           The command was successful.
**/
SHELL_STATUS
EFIAPI
ShellCommandRunUFM(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
);

#endif /* UFM_COMMAND_LIB_H_ */
