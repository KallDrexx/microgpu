#include <assert.h>
#include <SDL.h>
#include "sdl_display.h"
#include "microgpu-common/display.h"

void transfer_framebuffer(Mgpu_Display *display, Mgpu_FrameBuffer *frameBuffer) {
    Mgpu_Color *source = frameBuffer->pixels;
    uint32_t *target = display->pixelBuffer;
    uint16_t colPadding = display->width - (frameBuffer->width * frameBuffer->scale);
    uint16_t rowPadding = display->height - (frameBuffer->height * frameBuffer->scale);

    for (int row = 0; row < frameBuffer->height; row++) {
        for (int rowScale = 0; rowScale < frameBuffer->scale; rowScale++) {
            if (rowScale > 0) {
                // Reset back to the start of the line to duplicate the previous line.
                // A memcopy() is probably faster, but the point of sdl implementation isn't speed.
                source -= frameBuffer->width;
            }

            for (int col = 0; col < frameBuffer->width; col++) {
                for (int colScale = 0; colScale < frameBuffer->scale; colScale++) {
                    uint8_t red, green, blue;
                    mgpu_color_get_rgb888(*source, &red, &green, &blue);
                    uint32_t color = ((uint32_t) red << 16) | ((uint32_t) green << 8) | blue;
                    *target = color;
                    target++;
                }
                source++;
            }

            // Add any padding to make sure non-exact scaling doesn't cause
            // pixel shifting / starts next pixel on the correct row.
            for (int col = 0; col < colPadding; col++) {
                *target = 0; // black
                target++;
            }
        }
    }

    // Add row padding
    for (int row = 0; row < rowPadding; row++) {
        for (int col = 0; col < display->width; col++) {
            *target = 0; // black
            target++;
        }
    }
}

Mgpu_Display *mgpu_display_new(const Mgpu_Allocator *allocator, const Mgpu_DisplayOptions *options) {
    assert(allocator != NULL);

    Mgpu_Display *display = allocator->AllocateFn(sizeof(Mgpu_Display));
    display->allocator = allocator;
    if (display == NULL) {
        fprintf(stderr, "NULL pointer returned from allocation function.\n");
        return NULL;
    }

    display->width = options->width;
    display->height = options->height;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL: %s.\n", SDL_GetError());
        return NULL;
    }

    uint32_t window_flags = SDL_WINDOW_SHOWN;
    display->window = SDL_CreateWindow(
            "MicroGPU SDL Renderer",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            display->width,
            display->height,
            window_flags);

    if (!display->window) {
        fprintf(stderr, "Error creating window: %s\n", SDL_GetError());
        mgpu_display_free(display);

        return NULL;
    }

    display->renderer = SDL_CreateRenderer(display->window, -1, 0);
    if (!display->renderer) {
        fprintf(stderr, "Error creating renderer: %s\n", SDL_GetError());
        mgpu_display_free(display);

        return NULL;
    }

    display->pixelBuffer = malloc(sizeof(uint32_t) * display->height * display->width);
    if (display->pixelBuffer == NULL) {
        fprintf(stderr, "Error allocating pixel buffer\n");
        mgpu_display_free(display);

        return NULL;
    }

    display->texture = SDL_CreateTexture(
            display->renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            display->width,
            display->height);

    if (display->texture == NULL) {
        fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
        mgpu_display_free(display);

        return NULL;
    }

    return display;
}

void mgpu_display_free(Mgpu_Display *display) {
    if (display != NULL) {
        // SDL_DestroyRenderer destroys associated textures, so we shouldn't do that ourselves
        SDL_DestroyRenderer(display->renderer);
        SDL_DestroyWindow(display->window);
        display->allocator->FreeFn(display->pixelBuffer);
        display->pixelBuffer = NULL;
        display->allocator->FreeFn(display);
    }
}

void mgpu_display_get_dimensions(Mgpu_Display *display, uint16_t *width, uint16_t *height) {
    assert(display != NULL);
    assert(height != NULL);
    assert(width != NULL);

    *height = display->height;
    *width = display->width;
}

Mgpu_FrameBuffer *mgpu_display_render(Mgpu_Display *display, Mgpu_FrameBuffer *frameBuffer) {
    assert(display != NULL);
    assert(frameBuffer != NULL);
    assert(frameBuffer->width > 0);
    assert(frameBuffer->height > 0);

    transfer_framebuffer(display, frameBuffer);

    // Push the pixel buffer to the screen
    SDL_UpdateTexture(
            display->texture,
            NULL,
            display->pixelBuffer,
            (int) (display->width * sizeof(uint32_t)));

    SDL_RenderCopy(display->renderer, display->texture, NULL, NULL);
    SDL_RenderPresent(display->renderer);

    // Since we copied the frame buffer to a texture, and that texture is used for
    // SDL's window refresh cycle, we can immediately release the frame buffer
    return frameBuffer;
}
