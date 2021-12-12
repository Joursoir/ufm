#ifndef UFM_CMDS_H
#define UFM_CMDS_H

#include <Uefi.h>
#include <Library/ShellLib.h>

/*
 * Creates one or more directories.
 *
 * dir_name: the name of a directory or directories to create
 *
 * return: EFI_SUCCESS           Directory(-ies) was created
           EFI_INVALID_PARAMETER Parameter has an invalid value
           EFI_ACCESS_DENIED     Error while creating directory(-ies)
           EFI_OUT_OF_RESOURCES  Not enough resources were available to open the
                                 file
*/
EFI_STATUS make_directory(CONST CHAR16 *dir_name);

#endif /* UFM_CMDS_H */
