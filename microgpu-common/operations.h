#pragma once

#include <stdint.h>
#include "color.h"

/*
 * What type of operations are supported
 */
typedef enum {
    Mgpu_Operation_DrawRectangle = 1
} Mgpu_OperationType;

/*
 * Operation to draw a filled in rectangle to the framebuffer.
 */
typedef struct {
    uint16_t startX, startY, width, height;
    Mgpu_Color color;
} Mgpu_Op_DrawRectangle;

/*
 * Single type that can represent any type of operation that
 * the microgpu framework can support.
 */
typedef struct {
    Mgpu_OperationType type;
    union {
        Mgpu_Op_DrawRectangle draw_rectangle;
    };
} Mgpu_Operation;
