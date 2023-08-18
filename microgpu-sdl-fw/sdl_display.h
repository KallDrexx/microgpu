#pragma once

#include <stdint.h>
#include <SDL.h>

struct Mgpu_Display {
    uint16_t width, height;
    uint32_t *pixelBuffer;
    SDL_Texture *texture;
    SDL_Window *window;
    SDL_Renderer *renderer;
};
