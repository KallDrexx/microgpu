#pragma once

#include "alloc.h"
#include "color.h"

typedef struct {
    Mgpu_Color *pixels;
    uint16_t width;
    uint16_t height;
    uint8_t scale;
    const Mgpu_Allocator *allocator;
} Mgpu_FrameBuffer;

/*
 * Creates a new framebuffer based on the specified height and width (in pixels). The
 * frame buffer's width and height will be adjusted based on the passed in scale factor.
 * So if an original width and height of 320x240 is passed in with a scale factor of 2, then
 * the frame buffer will have a width and height 160x120.
 *
 * A scale factor of 1 or greater is required, a scale factor of zero is invalid.
 */
Mgpu_FrameBuffer *mgpu_framebuffer_new(uint16_t originalWidth,
                                       uint16_t originalHeight,
                                       uint8_t scaleFactor,
                                       const Mgpu_Allocator *allocator);

/*
 * Uninitializes the framebuffer and deallocates all memory used by it, including the framebuffer
 * pointer itself.
 */
void mgpu_framebuffer_free(Mgpu_FrameBuffer *frameBuffer);

/*
 * Clears the frame buffer to the specified color.
 */
void mgpu_framebuffer_clear(Mgpu_FrameBuffer *frameBuffer, Mgpu_Color color);
