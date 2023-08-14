#include "operations.h"
#include "drawing/rectangle.h"
#include "drawing/triangle.h"

void mgpu_execute_operation(Mgpu_Operation *operation, Mgpu_FrameBuffer *frameBuffer) {
    switch (operation->type) {
        case Mgpu_Operation_DrawRectangle:
            mgpu_draw_rectangle(&operation->drawRectangle, frameBuffer);
            break;

        case Mgpu_Operation_DrawTriangle:
            mgpu_draw_triangle(&operation->drawTriangle, frameBuffer);
            break;

        default:
            // todo: add error for un-executable operation
            break;
    }
}
