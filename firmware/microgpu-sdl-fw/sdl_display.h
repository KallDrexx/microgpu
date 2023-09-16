#pragma once

#include <stdint.h>
#include <SDL.h>
#include "microgpu-common/alloc.h"

struct Mgpu_Display {
    uint16_t width, height;
    uint32_t *pixelBuffer;
    SDL_Texture *texture;
    SDL_Window *window;
    SDL_Renderer *renderer;
    const Mgpu_Allocator *allocator;
};

struct Mgpu_DisplayOptions {
    uint16_t width, height;
};
