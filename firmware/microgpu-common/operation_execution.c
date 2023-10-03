#include <stdio.h>
#include "databus.h"
#include "messages.h"
#include "operations.h"
#include "microgpu-common/operations/batch.h"
#include "microgpu-common/operations/drawing/rectangle.h"
#include "microgpu-common/operations/drawing/triangle.h"
#include "microgpu-common/operations/get_last_message.h"
#include "microgpu-common/operations/present_framebuffer.h"
#include "microgpu-common/operations/reset.h"
#include "microgpu-common/operations/status.h"
#include "microgpu-common/operations/textures.h"

void mgpu_execute_operation(Mgpu_Operation *operation,
                            Mgpu_FrameBuffer *frameBuffer,
                            Mgpu_Display *display,
                            Mgpu_Databus *databus,
                            bool *resetFlag,
                            Mgpu_FrameBuffer **releasedFrameBuffer,
                            Mgpu_TextureManager *textureManager) {
    // Don't clear the last operation's message if the next operation
    // being requested is to get the latest message
    if (operation->type != Mgpu_Operation_GetLastMessage) {
        mgpu_message_set(NULL);
    }

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

        case Mgpu_Operation_GetLastMessage:
            mgpu_exec_get_last_message(databus);
            break;

        case Mgpu_Operation_PresentFramebuffer:
            mgpu_exec_present_framebuffer(display, frameBuffer, releasedFrameBuffer);
            break;

        case Mgpu_Operation_Batch:
            mgpu_exec_batch(&operation->batchOperation, frameBuffer, display, databus, resetFlag, releasedFrameBuffer,
                            textureManager);
            break;

        case Mgpu_Operation_Reset:
            mgpu_exec_reset(resetFlag);
            break;

        case Mgpu_Operation_SetTextureCount:
            mgpu_exec_texture_count(textureManager, &operation->setTextureCount);
            break;

        case Mgpu_Operation_DefineTexture:
            mgpu_exec_texture_define(textureManager, &operation->defineTexture);
            break;

        case Mgpu_Operation_AppendTexturePixels:
            mgpu_exec_texture_append(textureManager, &operation->appendTexturePixels);
            break;

        case Mgpu_Operation_RenderTexture:
            mgpu_exec_texture_render(textureManager, frameBuffer, &operation->drawTexture);
            break;

        default: {
            char buffer[MESSAGE_MAX_LEN];
            snprintf(buffer, MESSAGE_MAX_LEN, "Cannot execute operation of type %u", operation->type);
            mgpu_message_set(buffer);
            break;
        }
    }
}
