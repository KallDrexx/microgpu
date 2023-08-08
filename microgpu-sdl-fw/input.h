#ifndef MICROGPU_SDL_FW_INPUT_H
#define MICROGPU_SDL_FW_INPUT_H

#include <stdbool.h>

typedef struct {
    bool quit_requested: 1;

} Mgpu_Sdl_Input;

void mgpu_sdl_input_update(Mgpu_Sdl_Input *state);

#endif //MICROGPU_SDL_FW_INPUT_H
