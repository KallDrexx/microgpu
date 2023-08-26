#pragma once

/*
 * Defines which allocation and free functions should be used. This allows for customization
 * of different allocation strategies for different objects. Thus, the consumer could have
 * some objects allocated in external ram, internal ram, DMA capable ram, or even the stack.
 */
typedef struct {
    /*
     * Function to allocate memory of a certain size. Should return `NULL` if memory could
     * not be allocated for any reason.
     */
    void *(*const Mgpu_AllocateFn)(size_t);

    /*
     * Frees memory allocated with the corresponding `Mgpu_AllocateFn` call.
     */
    void (*const Mgpu_FreeFn)(void *);
} Mgpu_Allocator;