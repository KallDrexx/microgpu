#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>
#include "display.h"
#include "input.h"
#include "operations.h"

#define FPS 60
#define FRAME_TARGET_TIME (1000/FPS)

bool isRunning;
Mgpu_Sdl_Display display;
Mgpu_Sdl_Input input;
Mgpu_FrameBuffer framebuffer;

bool setup(void) {
    if (mgpu_sdl_display_init(&display) == false) {
        fprintf(stderr, "Failed to initialize display\n");
        return false;
    }

    size_t frameBufferBytes = mgpu_framebuffer_get_required_buffer_size(display.windowWidth, display.windowHeight);
    void *memory = malloc(frameBufferBytes);
    framebuffer = mgpu_framebuffer_new(memory, display.windowWidth, display.windowHeight);

    Mgpu_Op_DrawRectangle rectangle = {
            .startX = 200,
            .startY = 100,
            .width = 50,
            .height = 300,
            .color = mgpu_color_from_rgb888(255, 0, 0),
    };

    Mgpu_Operation operation = {
            .type = Mgpu_Operation_DrawRectangle,
            .drawRectangle = rectangle,
    };

    Mgpu_Op_DrawRectangle rectangle2 = {
            .startX = 0,
            .startY = 0,
            .width = 200,
            .height = 100,
            .color = mgpu_color_from_rgb888(0, 255, 0),
    };

    Mgpu_Operation operation2 = {
            .type = Mgpu_Operation_DrawRectangle,
            .drawRectangle = rectangle2,
    };

    Mgpu_Op_DrawRectangle rectangle3 = {
            .startX = 250,
            .startY = 400,
            .width = 500,
            .height = 500,
            .color = mgpu_color_from_rgb888(0, 0, 255),
    };

    Mgpu_Operation operation3 = {
            .type = Mgpu_Operation_DrawRectangle,
            .drawRectangle = rectangle3,
    };

    mgpu_execute_operation(&operation, &framebuffer);
    mgpu_execute_operation(&operation2, &framebuffer);
    mgpu_execute_operation(&operation3, &framebuffer);

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
        mgpu_sdl_apply_framebuffer(&display, &framebuffer);
        mgpu_sdl_push_to_screen(&display);
    }

    free(framebuffer.pixels);
    mgpu_sdl_display_uninit(&display);
}
