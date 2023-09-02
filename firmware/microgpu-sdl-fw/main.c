#define SDL_MAIN_HANDLED

#include <stdio.h>
#include <stdbool.h>
#include <SDL.h>
#include "microgpu-common/alloc.h"
#include "microgpu-common/operations.h"
#include "microgpu-common/operation_execution.h"

#if defined(DATABUS_BASIC)
#include "basic_databus.h"

#elif defined(DATABUS_TCP)
#include "tcp_databus.h"
#endif

#define FPS 60
#define FRAME_TARGET_TIME (1000/FPS)

static const Mgpu_Allocator basicAllocator = {
        .AllocateFn = malloc,
        .FreeFn = free,
};

bool isRunning, resetRequested;
Mgpu_Display *display;
Mgpu_Databus *databus;
uint16_t width, height;
Mgpu_FrameBuffer *framebuffer;
Mgpu_DatabusOptions dataBusOptions;

bool setup(void) {
#ifdef DATABUS_TCP
    dataBusOptions.port = 9123;
#endif

    databus = mgpu_databus_new(&dataBusOptions, &basicAllocator);
    if (databus == NULL) {
        fprintf(stderr, "Failed to initialize databus.\n");
        return false;
    }

    display = mgpu_display_new(&basicAllocator);
    if (display == NULL) {
        fprintf(stderr, "Failed to initialize display\n");
        return false;
    }

    return true;
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

int databus_loop(void *data) {
    Mgpu_Operation operation;
    while (isRunning) {
        if (mgpu_databus_get_next_operation(databus, &operation)) {
            mgpu_execute_operation(&operation, framebuffer, display, databus, &resetRequested);
#ifdef DATABUS_BASIC
            Mgpu_Response response;
            if (mgpu_basic_databus_get_last_response(databus, &response)) {
                handleResponse(&response);
            }
#endif
        }
    }

    mgpu_databus_free(databus);
    databus = NULL;

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
                mgpu_execute_operation(&operation, framebuffer, display, databus, &resetRequested);
#ifdef DATABUS_BASIC
                if (mgpu_basic_databus_get_last_response(databus, &response)) {
                    handleResponse(&response);
                }
#endif
            }
        }
    }

    mgpu_display_get_dimensions(display, &width, &height);
    framebuffer = mgpu_framebuffer_new(width, height, operation.initialize.frameBufferScale, &basicAllocator);
    if (framebuffer == NULL) {
        fprintf(stderr, "Framebuffer failed to be created\n");
        exit(1);
    }

    SDL_Log("Initialization operation applied\n");
}

void handle_sdl_keyup_event(SDL_Event *event) {
    switch ((*event).key.keysym.sym) {
        case SDLK_ESCAPE:
            isRunning = false;
            break;

        case SDLK_DELETE:
#ifdef DATABUS_BASIC
            mgpu_basic_databus_trigger_reset(databus);
#else
            resetRequested = true;
#endif
            break;
    }
}

void sdl_poll_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                isRunning = false;
                break;

            case SDL_KEYUP:
                handle_sdl_keyup_event(&event);
                break;
        }
    }
}

void start_sdl_system(void) {
    SDL_Thread *databusThread = NULL;

    SDL_Log("Starting SDL system\n");
    isRunning = setup();
    if (isRunning) {
        wait_for_init_op();
        databusThread = SDL_CreateThread(databus_loop, "Databus Loop", NULL);
    }

    uint32_t previousFrameTime = 0;
    while (isRunning) {
        if (resetRequested) {
            SDL_Log("Reset requested\n");
            isRunning = false;
            break;
        }

        int timeToWait = FRAME_TARGET_TIME - (SDL_GetTicks() - previousFrameTime);
        if (timeToWait > 0 && timeToWait <= FRAME_TARGET_TIME) {
            SDL_Delay(timeToWait);
        }

        sdl_poll_events();
    }

    SDL_Log("Waiting for databus to close\n");
    SDL_WaitThread(databusThread, NULL);

    SDL_Log("Finishing tear down\n");
    mgpu_framebuffer_free(framebuffer);
    framebuffer = NULL;

    mgpu_display_free(display);
    display = NULL;
}

int main(int argc, char *args[]) {
    SDL_Log("Color size: %u\n", sizeof(Mgpu_Color));

    while (true) {
        resetRequested = false;
        start_sdl_system();
        if (!resetRequested) {
            // User specifically requested closing but not a reset, so actually exit
            break;
        }
    }
}