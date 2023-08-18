#pragma once

#include <stdint.h>
#include "../color.h"
#include "../framebuffer.h"

/*
 * What type of operations are supported
 */
typedef enum {
    Mgpu_Operation_DrawRectangle = 1,
    Mgpu_Operation_DrawTriangle,
} Mgpu_OperationType;

/*
 * Operation to draw a filled in rectangle to the framebuffer.
 */
typedef struct {
    uint16_t startX, startY, width, height;
    Mgpu_Color color;
} Mgpu_Op_DrawRectangle;

/*
 * Operation to draw a filled in triangle between three defined points.
 */
typedef struct {
    uint16_t x0, y0, x1, y1, x2, y2;
    Mgpu_Color color;
} Mgpu_Op_DrawTriangle;

/*
 * Single type that can represent any type of operation that
 * the microgpu framework can support.
 */
typedef struct {
    Mgpu_OperationType type;
    union {
        Mgpu_Op_DrawRectangle drawRectangle;
        Mgpu_Op_DrawTriangle drawTriangle;
    };
} Mgpu_Operation;

void mgpu_execute_operation(Mgpu_Operation *operation, Mgpu_FrameBuffer *frameBuffer);
