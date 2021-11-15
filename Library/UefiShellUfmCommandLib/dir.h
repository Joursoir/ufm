#ifndef UFM_DIR_H
#define UFM_DIR_H

#include <Uefi.h>

struct dir_list {
	EFI_SHELL_FILE_INFO *list_head;
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

#endif /* UFM_DIR_H */
