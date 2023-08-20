#include "operations.h"
#include "databus.h"
#include "microgpu-common/operations/status.h"
#include "microgpu-common/operations/drawing/rectangle.h"
#include "microgpu-common/operations/drawing/triangle.h"

void mgpu_execute_operation(Mgpu_Operation *operation,
                            Mgpu_FrameBuffer *frameBuffer,
                            Mgpu_Display *display,
                            Mgpu_Databus *databus) {
    switch (operation->type) {
        case Mgpu_Operation_DrawRectangle:
            mgpu_draw_rectangle(&operation->drawRectangle, frameBuffer);
            break;

        case Mgpu_Operation_DrawTriangle:
            mgpu_draw_triangle(&operation->drawTriangle, frameBuffer);
            break;

        case Mgpu_Operation_GetStatus:
            mgpu_exec_status_op(display, frameBuffer, databus);
            break;

        default:
            // todo: add error for un-executable operation
            break;
    }
}
