#pragma once

#include "byte_buffer.h"

/*
 * Converts a raw RGB565 image to RLE encoding, using the specified color bytes as transparent.
 */
Mgpu_ByteBuffer *mgpu_texture_encoding_rle_encode(Mgpu_ByteBuffer *source,
                                                  Mgpu_Allocator *allocator,
                                                  uint16_t transparentColor);


