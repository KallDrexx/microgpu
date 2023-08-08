#include <SDL.h>
#include "input.h"

void handle_keyup_event(Mgpu_Sdl_Input *state, SDL_Event *event);

void mgpu_sdl_input_update(Mgpu_Sdl_Input *state) {
    state->quit_requested = false;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                state->quit_requested = true;
                break;

            case SDL_KEYUP:
                handle_keyup_event(state, &event);
                break;
        }
    }
}

void handle_keyup_event(Mgpu_Sdl_Input *state, SDL_Event *event) {
    switch ((*event).key.keysym.sym) {
        case SDLK_ESCAPE:
            state->quit_requested = true;
            break;
    }
}
