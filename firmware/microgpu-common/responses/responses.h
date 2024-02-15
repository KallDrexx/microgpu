#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "microgpu-common/colors/color.h"

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

    /* The maximum number of bytes that the gpu supports for each operation */
    uint16_t opByteLimit;

    /* Identifies the version of the GPU's API */
    uint16_t apiVersionId;
} Mgpu_StatusResponse;

/*
 * Sends the last lastMessage raised by the microgpu system, usually in response
 * to the latest operation that was requested.
 *
 * The lastMessage is a null terminated string.
 */
typedef struct {
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
