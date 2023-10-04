#pragma once

#include "alloc.h"

/*
 * Creates a new list for items of the specified size. Returns a pointer to the
 * first element in the list, or NULL if memory could not be allocated for the list.
 *
 * The list is created with a length of zero, meaning that the pointer returned may
 * point to invalid memory until the first insert occurs.
 */
void *mgpu_list_new(Mgpu_Allocator *allocator, size_t itemSize, size_t initialCapacity);

/*
 * Frees all memory allocated by the list
 */
void mgpu_list_free(void *list);

/*
 * Gets the number of items in the list
 */
size_t mgpu_list_length(void *list);

/*
 * Adds a new item at the specified index in the collection and returns a pointer
 * to that item's location. The new item will have its memory zeroed out. All items
 * at or after the specified index will be shifted downward in the list.
 *
 * If the list needs more capacity to accommodate the new item, then list will be
 * reallocated and the list pointer will change to the new location. All other pointers
 * to the list must be modified manually.
 *
 * If the list needs more capacity but reallocation fails, then the list stays in
 * its original memory location and NULL is returned.
 *
 * If the index provided is greater than the length, then the item will be appended
 * to the back of the list.
 */
void *mgpu_list_insert(void **list, size_t index);

/*
 * Removes the specified item from the list. Any pointers to the item being removed
 * should be cleared, as they will likely point to a different item due to shifting
 */
void mgpu_list_remove(void *list, size_t index);

/*
 * Clears all items from the list while keeping capacity. Note that the memory
 * for all items that existed is not zeroed out.
 */
void mgpu_list_clear(void *list);
