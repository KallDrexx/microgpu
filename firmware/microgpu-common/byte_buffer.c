#include <assert.h>
#include "byte_buffer.h"

Mgpu_ByteBuffer *mgpu_byte_buffer_new(Mgpu_Allocator *allocator, size_t byteCount, bool allocateInFastMemory) {
    assert(allocator != NULL);
    assert(byteCount > 0);

    Mgpu_ByteBuffer *buffer = allocateInFastMemory
                              ? allocator->FastMemAllocateFn(sizeof(Mgpu_ByteBuffer) + byteCount)
                              : allocator->SlowMemAllocateFn(sizeof(Mgpu_ByteBuffer) + byteCount);

    buffer->length = byteCount;
    buffer->allocator = allocator;
    buffer->allocatedInFastMemory = allocateInFastMemory;

    return buffer;
}

void mgpu_byte_buffer_free(Mgpu_ByteBuffer *buffer) {
    if (buffer != NULL) {
        assert(buffer->allocator != NULL);
        if (buffer->allocatedInFastMemory) {
            buffer->allocator->FastMemFreeFn(buffer);
        } else {
            buffer->allocator->SlowMemFreeFn(buffer);
        }
    }
}

void mgpu_byte_buffer_slice(Mgpu_ByteBuffer *buffer, size_t startIndex, size_t count, Mgpu_ByteSlice *destination) {
    assert(buffer != NULL);
    assert(destination != NULL);
    assert(startIndex + count < buffer->length);

    destination->length = count;
    destination->data = buffer->data + startIndex;
}
