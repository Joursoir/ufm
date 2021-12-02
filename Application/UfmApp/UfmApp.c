#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/ShellLib.h>
#include <Library/UfmCommandLib.h>
#include <Protocol/ShellParameters.h>

/**
  Application Entry Point wrapper around the shell command

  @param[in] ImageHandle  Handle to the Image (NULL if internal).
  @param[in] SystemTable  Pointer to the System Table (NULL if internal).
**/
EFI_STATUS
EFIAPI
UfmAppMain(
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable)
{
  return ShellCommandRunUFM(gImageHandle, SystemTable);
}
