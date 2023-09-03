#include <assert.h>
#include "framebuffer.h"

Mgpu_FrameBuffer *mgpu_framebuffer_new(uint16_t originalWidth,
                                       uint16_t originalHeight,
                                       uint8_t scaleFactor,
                                       const Mgpu_Allocator *allocator) {
    assert(allocator != NULL);
    assert(scaleFactor >= 1);

    Mgpu_FrameBuffer *frameBuffer = allocator->AllocateFn(sizeof(Mgpu_FrameBuffer));
    if (frameBuffer == NULL) {
        return NULL;
    }

    frameBuffer->allocator = allocator;
    frameBuffer->width = originalWidth / scaleFactor;
    frameBuffer->height = originalHeight / scaleFactor;
    frameBuffer->scale = scaleFactor;
    frameBuffer->pixels = allocator->AllocateFn(sizeof(Mgpu_Color) * frameBuffer->width * frameBuffer->height);
    if (frameBuffer->pixels == NULL) {
        mgpu_framebuffer_free(frameBuffer);
        return NULL;
    }

    for (size_t index = 0; index < frameBuffer->width * frameBuffer->height; index++) {
        frameBuffer->pixels[index] = mgpu_color_from_rgb888(0, 0, 0);
    }

    return frameBuffer;
}

void mgpu_framebuffer_free(Mgpu_FrameBuffer *frameBuffer) {
    if (frameBuffer != NULL) {
        if (frameBuffer->pixels != NULL) {
            frameBuffer->allocator->FreeFn(frameBuffer->pixels);
            frameBuffer->pixels = NULL;
        }

        frameBuffer->allocator->FreeFn(frameBuffer);
    }
}

void mgpu_framebuffer_clear(Mgpu_FrameBuffer *frameBuffer, Mgpu_Color color) {
    assert(frameBuffer != NULL);

    int pixelTotal = frameBuffer->width * frameBuffer->height;
    Mgpu_Color *pixel = frameBuffer->pixels;
    for (int x = 0; x < pixelTotal; x++) {
        *pixel = color;
        pixel++;
    }
}
