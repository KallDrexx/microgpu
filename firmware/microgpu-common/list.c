#include <stdint.h>
#include <assert.h>
#include "list.h"

typedef struct {
    Mgpu_Allocator *allocator;
    size_t itemSize;
    size_t capacity;
    size_t length;
    uint8_t data[];
} List;

List *get_list_root(void *list) {
    assert(list != NULL);

    // Since *list points to the address of the first element, we have to look at the bytes prior to that
    // address to get the underlying list structure
    return (List *) ((list) - sizeof(List));
}

void *mgpu_list_new(Mgpu_Allocator *allocator, size_t itemSize, size_t initialCapacity) {
    assert(allocator != NULL);
    assert(itemSize > 0);
    assert(initialCapacity > 0);

    List *list = allocator->AllocateFn(sizeof(List) + initialCapacity * itemSize);
    if (list == NULL) {
        return NULL;
    }

    list->allocator = allocator;
    list->itemSize = itemSize;
    list->capacity = initialCapacity;
    list->length = 0;

    return list->data;
}

void mgpu_list_free(void *list) {
    if (list != NULL) {
        List *root = get_list_root(list);
        root->allocator->FreeFn(root);
    }
}

size_t mgpu_list_length(void *list) {
    assert(list != NULL);

    List *root = get_list_root(list);
    return root->length;
}

void mgpu_list_clear(void *list) {
    assert(list != NULL);

    List *root = get_list_root(list);
    root->length = 0;
}

void *mgpu_list_insert(void **list, size_t index) {
    assert(list != NULL);
    assert(*list != NULL);

}

void mgpu_list_remove(void *list, size_t index) {

}


