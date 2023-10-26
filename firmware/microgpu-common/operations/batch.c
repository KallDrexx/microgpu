#include <stdio.h>
#include "microgpu-common/operation_execution.h"
#include "microgpu-common/operation_deserializer.h"
#include "batch.h"
#include "microgpu-common/messages.h"

void mgpu_exec_batch(Mgpu_BatchOperation *batchOperation,
                     Mgpu_FrameBuffer *frameBuffer,
                     Mgpu_Display *display,
                     Mgpu_Databus *databus,
                     bool *resetFlag,
                     Mgpu_FrameBuffer **releasedFrameBuffer,
                     Mgpu_TextureManager *textureManager) {
    assert(batchOperation != NULL);
    assert(frameBuffer != NULL);
    assert(databus != NULL);

    uint16_t outerBytesLeft = batchOperation->byteLength;
    const uint8_t *buffer = batchOperation->bytes;

    while (outerBytesLeft > 2) {
        uint16_t innerSize = (buffer[0] << 8) | buffer[1];

        // sanity check
        if (innerSize > outerBytesLeft - 2) {
            char msg[MESSAGE_MAX_LEN] = {0};
            snprintf(msg, MESSAGE_MAX_LEN, "Batch had inner size of %u but only %u bytes left", innerSize, outerBytesLeft);
            mgpu_message_set(msg);
            return;
        }

        Mgpu_Operation operation;
        if (!mgpu_operation_deserialize(buffer + 2, innerSize, &operation)) {
            // Deserialization failed, so we can't trust the rest of the batch
            return;
        }

        mgpu_execute_operation(&operation, frameBuffer, display, databus, resetFlag, releasedFrameBuffer,
                               textureManager);

        buffer += innerSize + 2;
        outerBytesLeft -= innerSize + 2;
    }
}
