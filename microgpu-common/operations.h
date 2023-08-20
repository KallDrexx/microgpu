#pragma once

#include <stdint.h>
#include "color.h"
#include "framebuffer.h"
#include "display.h"

/*
 * What type of operations are supported
 */
typedef enum {
    Mgpu_Operation_Initialize = 1,
    Mgpu_Operation_DrawRectangle,
    Mgpu_Operation_DrawTriangle,
    Mgpu_Operation_GetStatus,
    Mgpu_Operation_GetLastMessage,
} Mgpu_OperationType;

/*
 * Operation to initialize the microgpu system. Must be provided before
 * most other operations can operate and has no effect after a previous
 * initialization call.
 */
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

/*
 * Operation to draw a filled in rectangle to the framebuffer.
 */
typedef struct {
    uint16_t startX, startY, width, height;
    Mgpu_Color color;
} Mgpu_DrawRectangleOperation;

/*
 * Operation to draw a filled in triangle between three defined points.
 */
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
