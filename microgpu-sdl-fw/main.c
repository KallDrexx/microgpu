#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>
#include "microgpu-common/operations.h"
#include "microgpu-common/operation_execution.h"
#include "input.h"
#include "basic_databus.h"

#define FPS 60
#define FRAME_TARGET_TIME (1000/FPS)

bool isRunning;
Mgpu_Display *display;
Mgpu_Databus *databus;
uint16_t width, height;
Mgpu_Sdl_Input input;
Mgpu_FrameBuffer framebuffer;
struct Mgpu_DataBusOptions dataBusOptions;

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

    return true;
}

void process_input(void) {
    mgpu_sdl_input_update(&input);
    if (input.quit_requested) {
        isRunning = false;
    }
}

void handleResponse(Mgpu_Response *response) {
    switch (response->type) {
        case Mgpu_Response_Status:
            SDL_Log("Status received\n");
            SDL_Log("Display: %ux%u\n", response->status.displayWidth, response->status.displayHeight);
            SDL_Log("Framebuffer: %ux%u\n", response->status.frameBufferWidth, response->status.frameBufferHeight);
            SDL_Log("Color mode: %u\n", response->status.colorMode);
            SDL_Log("Is Initialized: %u\n\n", response->status.isInitialized);
            break;

        case Mgpu_Response_LastMessage:
            SDL_Log("GetLastMessage response received: %s", response->lastMessage.message);
            break;

        default:
            break;
    }
}

int databus_loop() {
    Mgpu_Operation operation;
    Mgpu_Response response;
    while (isRunning) {
        if (mgpu_databus_get_next_operation(databus, &operation)) {
            mgpu_execute_operation(&operation, &framebuffer, display, databus);
            if (mgpu_basic_databus_get_last_response(databus, &response)) {
                handleResponse(&response);
            }
        }
    }

    mgpu_databus_uninit(databus);
    free(databus);

    return 0;
}

void wait_for_init_op() {
    SDL_Log("Waiting for initialization operation\n");
    Mgpu_Operation operation;
    Mgpu_Response response;
    while (isRunning) {
        bool hasOperation = mgpu_databus_get_next_operation(databus, &operation);
        if (hasOperation) {
            if (operation.type == Mgpu_Operation_Initialize) {
                break;
            }

            if (operation.type == Mgpu_Operation_GetStatus || operation.type == Mgpu_Operation_GetLastMessage) {
                // Can't respond to other operations before initialization
                mgpu_execute_operation(&operation, &framebuffer, display, databus);
                if (mgpu_basic_databus_get_last_response(databus, &response)) {
                    handleResponse(&response);
                }
            }
        }
    }

    mgpu_display_get_dimensions(display, &width, &height);
    width /= operation.initialize.frameBufferScale;
    height /= operation.initialize.frameBufferScale;

    size_t frameBufferBytes = mgpu_framebuffer_get_required_buffer_size(width, height);
    void *memory = malloc(frameBufferBytes);
    framebuffer = mgpu_framebuffer_init(memory, width, height, operation.initialize.frameBufferScale);

    SDL_Log("Initialization operation applied\n");
}

int main(int argc, char *args[]) {
    isRunning = setup();
    if (isRunning) {
        wait_for_init_op();
        SDL_CreateThread(databus_loop, "Databus Loop", NULL);
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
