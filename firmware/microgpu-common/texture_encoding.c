#include <assert.h>
#include "texture_encoding.h"

size_t perform_encode(uint8_t *destination, const Mgpu_ByteBuffer *source, uint16_t transparentColor) {
    assert(source != NULL);

    // If destination is null, we are just trying to get a final byte count with no actual transfer.
    // We also assume that if destination is not null, we've validated it's long enough for the full encoding.
    size_t sourceIndex = 0, destinationIndex = 0;
    uint8_t transparentByte0 = (uint8_t) (transparentColor >> 8);
    uint8_t transparentByte1 = (uint8_t) (transparentColor | 0x00FF);

    while (sourceIndex < source->length) {
        // transparent pixels first
        uint8_t transparentCount = 0;
        uint8_t pixelCount = 0;
        while (sourceIndex < source->length - 1) {
            if (transparentCount >= 255 ||
                source->data[sourceIndex] != transparentByte0 ||
                source->data[sourceIndex + 1] != transparentByte1) {
                break;
            }

            transparentCount++;
            sourceIndex += 2;
        }

        while (sourceIndex < source->length - 1) {
            if (pixelCount >= 255 ||
                (source->data[sourceIndex] == transparentByte0 &&
                 source->data[sourceIndex + 1] == transparentByte1)) {
                break;
            }

            if (destination != NULL) {
                destination[destinationIndex + 2 + (pixelCount * 2)] = source->data[sourceIndex];
                destination[destinationIndex + 2 + (pixelCount * 2) + 1] = source->data[sourceIndex + 1];
            }

            pixelCount++;
            sourceIndex += 2;
        }

        if (destination != NULL) {
            destination[destinationIndex] = transparentCount;
            destination[destinationIndex + 1] = pixelCount;
        }

        destinationIndex += pixelCount + 2;
    }

    // Return number of bytes "written"
    return destinationIndex + 1;
}

Mgpu_ByteBuffer *mgpu_texture_encoding_rle_encode(Mgpu_ByteBuffer *source,
                                                  Mgpu_Allocator *allocator,
                                                  uint16_t transparentColor) {
    assert(source != NULL);
    assert(allocator != NULL);

    // We need to iterate through twice, the first time to figure out how much memory to allocate. Encoding
    // should be rare (especially outside benchmark/test projects) so it's probably fine.

    size_t sizeRequired = perform_encode(NULL, source, transparentColor);
    Mgpu_ByteBuffer *encodedBuffer = mgpu_byte_buffer_new(allocator, sizeRequired, false);

    perform_encode(encodedBuffer->data, source, transparentColor);

    return encodedBuffer;
}
