#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "color.h"

/*
 * The type of responses that can be sent to over the databus
 */
typedef enum {
    Mgpu_Response_Status = 1,
    Mgpu_Response_LastMessage,
} Mgpu_ResponseType;

/*
 * A status response declares the current operating state of the
 * microgpu system.
 */
typedef struct {
    bool isInitialized;
    uint16_t displayWidth, displayHeight, frameBufferWidth, frameBufferHeight;
    Mgpu_ColorMode colorMode;
} Mgpu_StatusResponse;

/*
 * Sends the last message raised by the microgpu system, usually in response
 * to the latest operation that was requested.
 *
 * The message is a null terminated string.
 */
typedef struct {
    bool isError;
    char *message;
} Mgpu_LastMessageResponse;

/*
 * The composed response to send over the databus.
 */
typedef struct {
    Mgpu_ResponseType type;
    union {
        Mgpu_StatusResponse status;
        Mgpu_LastMessageResponse lastMessage;
    };
} Mgpu_Response;
