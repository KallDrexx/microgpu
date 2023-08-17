#include <assert.h>
#include <SDL.h>
#include "sdl_display.h"

#define WINDOW_HEIGHT 480
#define WINDOW_WIDTH 640

typedef struct {
    uint16_t width, height;
    uint32_t *pixelBuffer;
    SDL_Texture *texture;
    SDL_Window *window;
    SDL_Renderer *renderer;
} Mgpu_Sdl_Display;

uint16_t mgpu_sdl_display_get_width(Mgpu_DisplayPtr display) {
    Mgpu_Sdl_Display *sdl_display = (Mgpu_Sdl_Display *) display;
    return sdl_display->width;
}

uint16_t mgpu_sdl_display_get_height(Mgpu_DisplayPtr display) {
    Mgpu_Sdl_Display *sdl_display = (Mgpu_Sdl_Display *) display;
    return sdl_display->height;
}

void mgpu_sdl_display_render_framebuffer(Mgpu_DisplayPtr display, Mgpu_FrameBuffer *frameBuffer) {
    Mgpu_Sdl_Display *sdl_display = (Mgpu_Sdl_Display *) display;

    assert(display != NULL);
    assert(frameBuffer != NULL);
    assert(frameBuffer->width == sdl_display->width);
    assert(frameBuffer->height == sdl_display->height);

    // Apply the frame buffer to the pixel buffer
    size_t pixelCount = frameBuffer->width * frameBuffer->height;
    for (size_t x = 0; x < pixelCount; x++) {
        uint8_t red, green, blue;
        mgpu_color_get_rgb888(frameBuffer->pixels[x], &red, &green, &blue);
        uint32_t color = ((uint32_t) red << 16) | ((uint32_t) green << 8) | blue;
        sdl_display->pixelBuffer[x] = color;
    }

    // Push the pixel buffer to the screen
    SDL_UpdateTexture(
            sdl_display->texture,
            NULL,
            sdl_display->pixelBuffer,
            (int) (sdl_display->width * sizeof(uint32_t)));

    SDL_RenderCopy(sdl_display->renderer, sdl_display->texture, NULL, NULL);
    SDL_RenderPresent(sdl_display->renderer);
}

static const Mgpu_DisplayVTable Mgpu_Sdl_DisplayVtable = {
        .get_height = mgpu_sdl_display_get_height,
        .get_width = mgpu_sdl_display_get_width,
        .render_framebuffer = mgpu_sdl_display_render_framebuffer,
};

Mgpu_Display *mgpu_sdl_display_create() {
    Mgpu_Sdl_Display *display = malloc(sizeof(Mgpu_Sdl_Display));
    Mgpu_Display *mgpuDisplay = malloc(sizeof(Mgpu_Display));
    mgpuDisplay->self = display;
    mgpuDisplay->vTable = &Mgpu_Sdl_DisplayVtable;

    display->width = WINDOW_WIDTH;
    display->height = WINDOW_HEIGHT;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL: %s.\n", SDL_GetError());
        return false;
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
        mgpu_sdl_display_free(mgpuDisplay);

        return NULL;
    }

    display->renderer = SDL_CreateRenderer(display->window, -1, 0);
    if (!display->renderer) {
        fprintf(stderr, "Error creating renderer: %s\n", SDL_GetError());
        mgpu_sdl_display_free(mgpuDisplay);

        return false;
    }

    display->pixelBuffer = malloc(sizeof(uint32_t) * display->height * display->width);
    if (display->pixelBuffer == NULL) {
        fprintf(stderr, "Error allocating pixel buffer\n");
        mgpu_sdl_display_free(mgpuDisplay);

        return false;
    }

    display->texture = SDL_CreateTexture(
            display->renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            display->width,
            display->height);

    if (display->texture == NULL) {
        fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
        mgpu_sdl_display_free(mgpuDisplay);

        return false;
    }

    return mgpuDisplay;
}

void mgpu_sdl_display_free(Mgpu_Display *display) {
    if (display != NULL) {
        Mgpu_Sdl_Display *sdl_display = (Mgpu_Sdl_Display *) display->self;

        // SDL_DestroyRenderer destroys associated textures, so we shouldn't do that ourselves
        SDL_DestroyRenderer(sdl_display->renderer);
        SDL_DestroyWindow(sdl_display->window);
        free(sdl_display->pixelBuffer);

        sdl_display = NULL;

        display->self = NULL;
        display->vTable = NULL;
        free(display);
    }
}
