#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>
#include "display.h"
#include "input.h"
#include "color.h"
#include "framebuffer.h"

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

    for (size_t row = 0; row < framebuffer.height; row++) {
        for (size_t col = 0; col < framebuffer.width; col++) {
            size_t index = row * framebuffer.width + col;
            switch ((col / 20) % 3) {
                case 0:
                    framebuffer.pixels[index] = mgpu_color_from_rgb888(255,0, 0);
                    break;

                case 1:
                    framebuffer.pixels[index] = mgpu_color_from_rgb888(0,255, 0);
                    break;

                case 2:
                    framebuffer.pixels[index] = mgpu_color_from_rgb888(0,0, 255);
                    break;
            }
        }
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
        mgpu_sdl_apply_framebuffer(&display, &framebuffer);
        mgpu_sdl_push_to_screen(&display);
    }

    free(framebuffer.pixels);
    mgpu_sdl_display_uninit(&display);
}
