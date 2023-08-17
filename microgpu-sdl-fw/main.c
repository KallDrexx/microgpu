#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>
#include "sdl_display.h"
#include "input.h"
#include "operations.h"

#define FPS 60
#define FRAME_TARGET_TIME (1000/FPS)

bool isRunning;
Mgpu_Display *display;
uint16_t width, height;
Mgpu_Sdl_Input input;
Mgpu_FrameBuffer framebuffer;

bool setup(void) {
    display = mgpu_sdl_display_create();
    if (display == NULL) {
        fprintf(stderr, "Failed to initialize display\n");
        return false;
    }

    width = mgpu_display_get_width(display);
    height = mgpu_display_get_height(display);

    size_t frameBufferBytes = mgpu_framebuffer_get_required_buffer_size(width, height);
    void *memory = malloc(frameBufferBytes);
    framebuffer = mgpu_framebuffer_new(memory, width, height);

    Mgpu_Op_DrawTriangle triangleOperation = {
            .color = mgpu_color_from_rgb888(255, 0, 0),
            .x0 = 20, .y0 = 440,
            .x1 = 620, .y1 = 30,
            .x2 = 600, .y2 = 400,
    };

    Mgpu_Operation operation = {
            .type = Mgpu_Operation_DrawTriangle,
            .drawTriangle = triangleOperation,
    };

    mgpu_execute_operation(&operation, &framebuffer);

    return true;
}

void process_input(void) {
    mgpu_sdl_input_update(&input);
    if (input.quit_requested) {
        isRunning = false;
    }
}

int main(int argc, char *args[]) {
    isRunning = setup();

    uint32_t previousFrameTime = 0;
    while (isRunning) {
        int timeToWait = FRAME_TARGET_TIME - (SDL_GetTicks() - previousFrameTime);
        if (timeToWait > 0 && timeToWait <= FRAME_TARGET_TIME) {
            SDL_Delay(timeToWait);
        }

        process_input();
        mgpu_display_render_framebuffer(display, &framebuffer);
    }

    free(framebuffer.pixels);
    mgpu_sdl_display_free(display);
}
