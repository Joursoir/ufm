#ifndef UFM_FS_H
#define UFM_FS_H

#include <Uefi.h>

struct fs_array {
	CHAR16 **full_name;
	int len; // number of elements in array
};

/*
 * Gets the mapped names of the filesystems
 *
 * handles: a pointer to the buffer to array of handles that support Simple FS Protocol
 * count: the number of handles
 *
 * return: A pointer to the allocated structure or NULL if allocation fails
*/
struct fs_array *fsa_alloc(EFI_HANDLE *handles, UINTN count);

/*
 * Deletes the array of filesystems, frees the structure
 *
 * fsa: the pointer to the array of filesystems
 *
 * return: VOID
*/
VOID fsa_release(struct fs_array *fsa);

/*
 * Gets names of filesystems represented on the system
 *
 * return: A pointer to the allocated structure or NULL if allocation fails
*/
struct fs_array *scanfs(VOID);

#endif /* UFM_FS_H */
