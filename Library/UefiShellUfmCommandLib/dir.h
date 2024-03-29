#ifndef UFM_DIR_H
#define UFM_DIR_H

#include <Uefi.h>

struct dir_list {
	EFI_SHELL_FILE_INFO *list_head;
    BOOLEAN *marked; // files are marked on a panel
	int len; // number of elements in list
};

/*
 * Opens a directory and gets all its entries
 *
 * search_path: the pointer to path string
 * attr: required file attributes
 *
 * return: A pointer to the allocated structure or NULL if allocation fails
*/
struct dir_list *dirl_alloc(CHAR16 *search_path, CONST UINT64 attr);

/*
 * Deletes the directory list, frees the structure
 *
 * dl: the pointer to the directory list
 *
 * return: VOID
*/
VOID dirl_release(struct dir_list *dl);

/*
 * Opens a directory and gets all its matching entries. This function 
 * supports wildcards and will process '?' and '*' as such
 * P.S: it's frontend function of dirl_alloc()
 *
 * search_path: the pointer to path string
 * wildcard: the pointer to wildcard string
 * attr: required file attributes
 *
 * return: A pointer to the allocated structure or NULL if allocation fails
*/
struct dir_list *scandir(CONST CHAR16 *search_path, CONST CHAR16 *wildcard,
    CONST UINT64 attr);

/*
 * Gets the data stored in the specified node of directory linked list
 *
 * dl: the pointer to the directory list
 * n: the node number (starts with 1)
 *
 * return: A pointer to the EFI_SHELL_FILE_INFO or NULL if search fails
*/
EFI_SHELL_FILE_INFO *dirl_getn(struct dir_list *dl, UINTN n);

#endif /* UFM_DIR_H */
