#pragma once

#include <stdint.h>
#include "color.h"
#include "framebuffer.h"
#include "display.h"

/*
 * What type of operations are supported
 */
typedef enum {
    /*
     * Operation to initialize the microgpu system. Must be provided before
     * most other operations can operate and has no effect after a previous
     * initialization call.
     */
    Mgpu_Operation_Initialize = 1,

    /*
     * Operation to draw a filled in rectangle to the framebuffer.
     */
    Mgpu_Operation_DrawRectangle,

    /*
     * Operation to draw a filled in triangle between three defined points.
     */
    Mgpu_Operation_DrawTriangle,

    /*
     * Requests getting the status of the microgpu system. Can be requested
     * prior to initialization, which is useful for getting the display
     * resolution to compute framebuffer scale.
     */
    Mgpu_Operation_GetStatus,

    /*
     * Requests the latest message raised by the microgpu system, which usually
     * corresponds to the previously executed operation.
     */
    Mgpu_Operation_GetLastMessage,

    /*
     * Sends the frame buffer to the display for presentation.
     */
    Mgpu_Operation_PresentFramebuffer,

    /*
     * Requests the microgpu to initialize itself and fully reset itself.
     */
    Mgpu_Operation_Reset = 189, // Higher value that's hard to see accidentally
} Mgpu_OperationType;

typedef struct {
    /*
     * How much to scale the frame buffer down from the display resolution.
     * E.g. if the display is 320x240 and a scale of 2 is provided, the frame
     * buffer will have a resolution of 160x120, and will be scaled up to 320x240
     * at render time.
     *
     * A scale factor of 0 is invalid
     */
    uint8_t frameBufferScale;
} Mgpu_InitializeOperation;

typedef struct {
    uint16_t startX, startY, width, height;
    Mgpu_Color color;
} Mgpu_DrawRectangleOperation;

typedef struct {
    uint16_t x0, y0, x1, y1, x2, y2;
    Mgpu_Color color;
} Mgpu_DrawTriangleOperation;

/*
 * Single type that can represent any type of operation that
 * the microgpu framework can support.
 */
typedef struct {
    Mgpu_OperationType type;
    union {
        Mgpu_InitializeOperation initialize;
        Mgpu_DrawRectangleOperation drawRectangle;
        Mgpu_DrawTriangleOperation drawTriangle;
    };
} Mgpu_Operation;
