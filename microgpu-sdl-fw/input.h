#pragma once

#include <stdbool.h>

typedef struct {
    bool quit_requested: 1;

} Mgpu_Sdl_Input;

void mgpu_sdl_input_update(Mgpu_Sdl_Input *state);
