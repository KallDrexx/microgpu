#pragma once

#include <stddef.h>
#include <assert.h>

/*
 * Defines which allocation and free functions should be used. This allows for customization
 * of different allocation strategies for different objects. Thus, the consumer could have
 * some objects allocated in external ram, internal ram, DMA capable ram, or even the stack.
 */
typedef struct {
    /*
     * Function to allocate memory of a certain size from a fast memory pool. Should return `NULL` if memory could
     * not be allocated for any reason.
     */
    void *(*const FastMemAllocateFn)(size_t size);

    /*
     * Frees memory allocated with the corresponding `FastMemAllocateFn` call.
     */
    void (*const FastMemFreeFn)(void *memory);

    /*
     * Function to allocate memory of a certain size from memory pool that's potentially slower than the "fast" memory
     * pool. Should return `NULL` if memory could not be allocated for any reason.
     */
    void *(*const SlowMemAllocateFn)(size_t size);

    /*
     * Frees memory allocated with the corresponding `SlowMemAllocateFn` call.
     */
    void (*const SlowMemFreeFn)(void *memory);
} Mgpu_Allocator;

static inline void mgpu_alloc_assert(const Mgpu_Allocator *allocator) {
    assert(allocator != NULL);
    assert(allocator->FastMemAllocateFn != NULL);
    assert(allocator->FastMemFreeFn != NULL);
    assert(allocator->SlowMemAllocateFn != NULL);
    assert(allocator->SlowMemFreeFn != NULL);
}