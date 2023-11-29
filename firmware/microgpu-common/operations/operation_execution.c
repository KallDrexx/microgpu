#include <stdio.h>
#include "microgpu-common/messages.h"
#include "operations.h"
#include "microgpu-common/operations/execution/batch.h"
#include "microgpu-common/operations/execution//drawing/rectangle.h"
#include "microgpu-common/operations/execution//drawing/triangle.h"
#include "microgpu-common/operations/execution/get_last_message.h"
#include "microgpu-common/operations/execution/present_framebuffer.h"
#include "microgpu-common/operations/execution/reset.h"
#include "microgpu-common/operations/execution//status.h"
#include "microgpu-common/operations/execution/textures.h"

void mgpu_execute_operation(Mgpu_Operation *operation,
                            Mgpu_Display *display,
                            Mgpu_Databus *databus,
                            bool *resetFlag,
                            Mgpu_TextureManager *textureManager) {
    assert(operation != NULL);
    assert(display != NULL);
    assert(databus != NULL);
    assert(textureManager != NULL);

    // Don't clear the last operation's message if the next operation
    // being requested is to get the latest message
    if (operation->type != Mgpu_Operation_GetLastMessage) {
        char *message = mgpu_message_get_pointer();
        assert(message != NULL);
        message[0] = '\0';
    }

    switch (operation->type) {
        case Mgpu_Operation_DrawRectangle:
            mgpu_draw_rectangle(&operation->drawRectangle, textureManager);
            break;

        case Mgpu_Operation_DrawTriangle:
            mgpu_draw_triangle(&operation->drawTriangle, textureManager);
            break;

        case Mgpu_Operation_GetStatus:
            mgpu_exec_status_op(display, textureManager, databus);
            break;

        case Mgpu_Operation_GetLastMessage:
            mgpu_exec_get_last_message(databus);
            break;

        case Mgpu_Operation_PresentFramebuffer:
            mgpu_exec_present_framebuffer(display, textureManager);
            break;

        case Mgpu_Operation_Batch:
            mgpu_exec_batch(&operation->batchOperation,
                            display,
                            databus,
                            resetFlag,
                            textureManager);
            break;

        case Mgpu_Operation_Reset:
            mgpu_exec_reset(resetFlag);
            break;

        case Mgpu_Operation_DefineTexture:
            mgpu_exec_texture_define(textureManager, &operation->defineTexture);
            break;

        case Mgpu_Operation_AppendTexturePixels:
            mgpu_exec_texture_append(textureManager, &operation->appendTexturePixels);
            break;

        case Mgpu_Operation_DrawTexture:
            mgpu_exec_texture_draw(textureManager, &operation->drawTexture);
            break;

        default: {
            char *message = mgpu_message_get_pointer();
            assert(message != NULL);
            snprintf(message, MESSAGE_MAX_LEN, "Cannot execute operation of type %u", operation->type);

            break;
        }
    }
}
