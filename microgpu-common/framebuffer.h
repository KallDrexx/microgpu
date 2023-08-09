#pragma once

#include "color.h"

typedef struct {
    Mgpu_Color *pixels;
    uint16_t width;
    uint16_t height;
} Mgpu_FrameBuffer;

/*
 * Returns the number of bytes that need to be allocated for the frame buffer's
 * pixel data based on the number of horizontal and vertical pixels for the
 * framebuffer. This can be used by the callers to allocate the memory however
 * they feel (stack or heap).
 */
size_t mgpu_framebuffer_get_required_buffer_size(uint16_t width, uint16_t height);

/*
 * Creates a new framebuffer using the provided memory area for the pixel buffer.
 * The memory pointer passed in is assumed to be correctly pre-allocated based
 * on the size returned in `mgpu_framebuffer_get_required_buffer_size()`. It also
 * assumes that the width and height provided is the same as well.
 *
 * The frame buffer will be initialized to all black pixels.
 *
 * The caller assumes responsibility of freeing the buffer once they discard the
 * framebuffer.
 */
Mgpu_FrameBuffer mgpu_framebuffer_new(void *memory, uint16_t width, uint16_t height);
