#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>
#include "microgpu-common/operations.h"
#include "microgpu-common/operation_execution.h"
#include "input.h"
#include "null_databus.h"

#define FPS 60
#define FRAME_TARGET_TIME (1000/FPS)

bool isRunning;
Mgpu_Display *display;
Mgpu_Databus *databus;
uint16_t width, height;
Mgpu_Sdl_Input input;
Mgpu_FrameBuffer framebuffer;
struct Mgpu_DataBusOptions dataBusOptions;

void executeTestOps(void) {
    Mgpu_DrawTriangleOperation triangleOperation = {
            .color = mgpu_color_from_rgb888(255, 0, 0),
            .x0 = 20, .y0 = 440,
            .x1 = 620, .y1 = 30,
            .x2 = 600, .y2 = 400,
    };

    Mgpu_Operation operation = {
            .type = Mgpu_Operation_DrawTriangle,
            .drawTriangle = triangleOperation,
    };

    mgpu_execute_operation(&operation, &framebuffer, display, databus);

    Mgpu_Operation statusOp = {.type = Mgpu_Operation_GetStatus};
    mgpu_execute_operation(&statusOp, &framebuffer, display, databus);

    Mgpu_Response response;
    mgpu_null_databus_get_last_response(databus, &response);

    assert(response.type == Mgpu_Response_Status);

    SDL_Log("Status received\n");
    SDL_Log("Display: %ux%u\n", response.status.displayWidth, response.status.displayHeight);
    SDL_Log("Framebuffer: %ux%u\n", response.status.frameBufferWidth, response.status.frameBufferHeight);
    SDL_Log("Color mode: %u\n", response.status.colorMode);
    SDL_Log("Is Initialized: %u\n", response.status.isInitialized);
}

bool setup(void) {
    size_t databusSize = mgpu_databus_get_size((Mgpu_DataBusOptions *) &dataBusOptions);
    void *memory = malloc(databusSize);
    if (memory == NULL) {
        fprintf(stderr, "Failed to initialize initial databus memory\n");
        return false;
    }

    databus = mgpu_databus_init(memory, (Mgpu_DataBusOptions *) &dataBusOptions);

    size_t displaySize = mgpu_display_get_size();
    memory = malloc(displaySize);
    if (memory == NULL) {
        fprintf(stderr, "Failed to initialize initial display memory\n");
        return false;
    }

    display = mgpu_display_init(memory);
    if (display == NULL) {
        fprintf(stderr, "Failed to initialize display\n");
        return false;
    }

    mgpu_display_get_dimensions(display, &width, &height);
    size_t frameBufferBytes = mgpu_framebuffer_get_required_buffer_size(width, height);
    memory = malloc(frameBufferBytes);
    framebuffer = mgpu_framebuffer_new(memory, width, height);

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
    if (isRunning) {
        executeTestOps();
    }

    uint32_t previousFrameTime = 0;
    while (isRunning) {
        int timeToWait = FRAME_TARGET_TIME - (SDL_GetTicks() - previousFrameTime);
        if (timeToWait > 0 && timeToWait <= FRAME_TARGET_TIME) {
            SDL_Delay(timeToWait);
        }

        process_input();
        mgpu_display_render(display, &framebuffer);
    }

    free(framebuffer.pixels);
    mgpu_display_uninit(display);
    free(display);
}
