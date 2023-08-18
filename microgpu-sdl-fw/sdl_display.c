#include <assert.h>
#include <SDL.h>
#include "sdl_display.h"
#include "microgpu-common/display.h"

#define WINDOW_HEIGHT 480
#define WINDOW_WIDTH 640

size_t mgpu_display_get_size() {
    // Since using SDL guarantees we are running on pc, and thus have more
    // malloc freedom, we'll malloc all the internal elements ourselves,
    // especially since we won't know the size of most SDL types afaik.
    return sizeof(Mgpu_Display);
}

Mgpu_Display *mgpu_display_init(void *memory) {
    assert(memory != NULL);

    Mgpu_Display *display = (Mgpu_Display *) memory;
    display->width = WINDOW_WIDTH;
    display->height = WINDOW_HEIGHT;

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
        mgpu_display_uninit(display);

        return NULL;
    }

    display->renderer = SDL_CreateRenderer(display->window, -1, 0);
    if (!display->renderer) {
        fprintf(stderr, "Error creating renderer: %s\n", SDL_GetError());
        mgpu_display_uninit(display);

        return NULL;
    }

    display->pixelBuffer = malloc(sizeof(uint32_t) * display->height * display->width);
    if (display->pixelBuffer == NULL) {
        fprintf(stderr, "Error allocating pixel buffer\n");
        mgpu_display_uninit(display);

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
        mgpu_display_uninit(display);

        return NULL;
    }

    return display;
}

void mgpu_display_uninit(Mgpu_Display *display) {
    if (display != NULL) {
        // SDL_DestroyRenderer destroys associated textures, so we shouldn't do that ourselves
        SDL_DestroyRenderer(display->renderer);
        SDL_DestroyWindow(display->window);
        free(display->pixelBuffer);
    }
}

void mgpu_display_get_dimensions(Mgpu_Display *display, uint16_t *width, uint16_t *height) {
    assert(display != NULL);
    assert(height != NULL);
    assert(width != NULL);

    *height = display->height;
    *width = display->width;
}

void mgpu_display_render(Mgpu_Display *display, Mgpu_FrameBuffer *frameBuffer) {
    assert(display != NULL);
    assert(frameBuffer != NULL);
    assert(frameBuffer->width == display->width);
    assert(frameBuffer->height == display->height);

    // Apply the frame buffer to the pixel buffer
    size_t pixelCount = frameBuffer->width * frameBuffer->height;
    for (size_t x = 0; x < pixelCount; x++) {
        uint8_t red, green, blue;
        mgpu_color_get_rgb888(frameBuffer->pixels[x], &red, &green, &blue);
        uint32_t color = ((uint32_t) red << 16) | ((uint32_t) green << 8) | blue;
        display->pixelBuffer[x] = color;
    }

    // Push the pixel buffer to the screen
    SDL_UpdateTexture(
            display->texture,
            NULL,
            display->pixelBuffer,
            (int) (display->width * sizeof(uint32_t)));

    SDL_RenderCopy(display->renderer, display->texture, NULL, NULL);
    SDL_RenderPresent(display->renderer);
}
