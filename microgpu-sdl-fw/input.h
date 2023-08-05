#ifndef MICROGPU_SDL_FW_INPUT_H
#define MICROGPU_SDL_FW_INPUT_H

#include <stdbool.h>

typedef struct {
    bool quit_requested: 1;

} MGPU_SDL_InputState;

void mgpu_sdl_input_update(MGPU_SDL_InputState *state);

#endif //MICROGPU_SDL_FW_INPUT_H
