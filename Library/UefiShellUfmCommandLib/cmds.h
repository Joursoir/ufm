#ifndef UFM_CMDS_H
#define UFM_CMDS_H

#include <Uefi.h>
#include <Library/ShellLib.h>

/*
 * Deletes a node including subdirectories
 *
 * node: the node to start deleting with
 *
 * return: EFI_SUCCESS           The operation was successful
           EFI_ACCESS_DENIED     A file was read only
           EFI_ABORTED           The abort was received
           EFI_DEVICE_ERROR      A device error occurred reading this node
*/
EFI_STATUS delete_file(EFI_SHELL_FILE_INFO *node);

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
