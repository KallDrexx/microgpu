#include <stdlib.h>
#include "operations.h"

void mgpu_draw_rectangle(Mgpu_Op_DrawRectangle *drawRectangle, Mgpu_FrameBuffer *frameBuffer);

void mgpu_execute_operation(Mgpu_Operation *operation, Mgpu_FrameBuffer *frameBuffer) {
    switch (operation->type) {
        case Mgpu_Operation_DrawRectangle:
            mgpu_draw_rectangle(&operation->draw_rectangle, frameBuffer);
            break;

        default:
            // todo: add error for un-executable operation
            break;
    }
}

void mgpu_draw_rectangle(Mgpu_Op_DrawRectangle *drawRectangle, Mgpu_FrameBuffer *frameBuffer) {
    if (drawRectangle->startX >= frameBuffer->width || drawRectangle->startY >= frameBuffer->height) {
        // It starts out of bounds, so nothing to draw.
        return;
    }

    uint16_t endX = min(drawRectangle->startX + drawRectangle->width, frameBuffer->width);
    uint16_t endY = min(drawRectangle->startY + drawRectangle->height, frameBuffer->height);
    uint16_t adjustedWidth = endX - drawRectangle->startX;
    uint16_t adjustedHeight = endY - drawRectangle->startY;

    Mgpu_Color *pixel = frameBuffer->pixels + ((drawRectangle->startY * frameBuffer->width) + drawRectangle->startX);
    for (uint16_t row = 0; row < adjustedHeight; row++) {
        for (uint16_t col = 0; col < adjustedWidth; col++) {
            *pixel = drawRectangle->color;
            pixel++;
        }

        pixel += frameBuffer->width - adjustedWidth;
    }
}
