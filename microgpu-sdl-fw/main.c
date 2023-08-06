#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>
#include "display.h"
#include "input.h"
#include "color.h"

#define FPS 60
#define FRAME_TARGET_TIME (1000/FPS)

bool isRunning;
MGPU_SDL_Display display;
MGPU_SDL_InputState input;

bool setup(void) {
    if (mgpu_sdl_display_init(&display) == false) {
        fprintf(stderr, "Failed to initialize display\n");
        return false;
    }

    return true;
}

void process_input(void) {
    mgpu_sdl_input_update(&input);
    if (input.quit_requested) {
        isRunning = false;
    }
}

int main(int argc, char* args[]) {
    isRunning = setup();

    uint32_t previousFrameTime = 0;
    while (isRunning) {
        int timeToWait = FRAME_TARGET_TIME - (SDL_GetTicks() - previousFrameTime);
        if (timeToWait > 0 && timeToWait <= FRAME_TARGET_TIME) {
            SDL_Delay(timeToWait);
        }

        process_input();
    }

    mgpu_sdl_display_uninit(&display);
}
