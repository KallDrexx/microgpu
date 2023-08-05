#include <assert.h>
#include <SDL.h>
#include "display.h"

#define WINDOW_HEIGHT 480
#define WINDOW_WIDTH 640

struct MGPU_SDL_Display_Internal {
    SDL_Texture *texture;
    SDL_Window *window;
    SDL_Renderer *renderer;
};

bool mgpu_sdl_display_init(MGPU_SDL_Display *display) {
    assert(display != NULL);

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "Error initializing SDL: %s.\n", SDL_GetError());
        return false;
    }

    display->internal = malloc(sizeof(struct MGPU_SDL_Display_Internal));;

    display->windowWidth = WINDOW_WIDTH;
    display->windowHeight = WINDOW_HEIGHT;

    uint32_t window_flags = SDL_WINDOW_SHOWN;
    display->internal->window = SDL_CreateWindow(
            "MicroGPU SDL Renderer",
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            display->windowWidth,
            display->windowHeight,
            window_flags);

    if (!display->internal->window) {
        fprintf(stderr, "Error creating window: %s\n", SDL_GetError());
        mgpu_sdl_display_uninit(display);

        return false;
    }

    display->internal->renderer = SDL_CreateRenderer(display->internal->window, -1, 0);
    if (!display->internal->renderer) {
        fprintf(stderr, "Error creating renderer: %s\n", SDL_GetError());
        mgpu_sdl_display_uninit(display);

        return false;
    }

    display->pixelBuffer = malloc(sizeof(uint32_t) * display->windowHeight * display->windowWidth);
    if (display->pixelBuffer == NULL) {
        fprintf(stderr, "Error allocating pixel buffer\n");
        mgpu_sdl_display_uninit(display);

        return false;
    }

    display->internal->texture = SDL_CreateTexture(
            display->internal->renderer,
            SDL_PIXELFORMAT_ARGB8888,
            SDL_TEXTUREACCESS_STREAMING,
            display->windowWidth,
            display->windowHeight);

    if (display->internal->texture == NULL) {
        fprintf(stderr, "Failed to create texture: %s\n", SDL_GetError());
        mgpu_sdl_display_uninit(display);

        return false;
    }

    return display;
}

void mgpu_sdl_display_uninit(MGPU_SDL_Display *display) {
    if (display != NULL) {
        if (display->internal != NULL) {
            // SDL_DestroyRenderer destroys associated textures, so we shouldn't do that ourselves
            SDL_DestroyRenderer(display->internal->renderer);
            SDL_DestroyWindow(display->internal->window);
            free(display->internal);
            display->internal = NULL;
        }

        free(display->pixelBuffer);
        display->pixelBuffer = NULL;
    }
}

void mgpu_sdl_push_to_screen(MGPU_SDL_Display *display) {
    assert(display != NULL);
    assert(display->internal != NULL);

    SDL_UpdateTexture(
            display->internal->texture,
            NULL,
            display->pixelBuffer,
            (int)(display->windowWidth * sizeof(uint32_t)));

    SDL_RenderCopy(display->internal->renderer, display->internal->texture, NULL, NULL);
    SDL_RenderPresent(display->internal->renderer);
}
