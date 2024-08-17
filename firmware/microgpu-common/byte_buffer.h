#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "alloc.h"

typedef struct {
    size_t length;
    Mgpu_Allocator *allocator;
    bool allocatedInFastMemory;
    uint8_t data[];
} Mgpu_ByteBuffer;

typedef struct {
    size_t length;
    uint8_t *data;
} Mgpu_ByteSlice;

/*
 * Creates a new byte buffer with the specified size. Data in buffer is not guaranteed to be zeroed out.
 */
Mgpu_ByteBuffer *mgpu_byte_buffer_new(Mgpu_Allocator *allocator, size_t byteCount, bool allocateInFastMemory);

/*
 * Frees memory allocated by a byte buffer
 */
void mgpu_byte_buffer_free(Mgpu_ByteBuffer *buffer);

/*
 * Updates the destination slice with a pointer to a subset of data within the original buffer.
 * Will panic if the start + count is out of bounds of the original buffer.
 */
void mgpu_byte_buffer_slice(Mgpu_ByteBuffer *buffer, size_t startIndex, size_t count, Mgpu_ByteSlice *destination);
