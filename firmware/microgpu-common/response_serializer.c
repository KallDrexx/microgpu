#include <assert.h>
#include "response_serializer.h"

int serialize_status(Mgpu_StatusResponse *status, uint8_t buffer[], size_t bufferSize) {
    assert(status != NULL);
    size_t requiredSize = 11;

    if (bufferSize < requiredSize) {
        return MGPU_ERROR_BUFFER_TOO_SMALL;
    }

    buffer[0] = Mgpu_Response_Status;
    buffer[1] = status->isInitialized;
    buffer[2] = status->displayWidth >> 8;
    buffer[3] = status->displayWidth & 0xFF;
    buffer[4] = status->displayHeight >> 8;
    buffer[5] = status->displayHeight & 0xFF;
    buffer[6] = status->frameBufferWidth >> 8;
    buffer[7] = status->frameBufferWidth & 0xFF;
    buffer[8] = status->frameBufferHeight >> 8;
    buffer[9] = status->frameBufferHeight & 0xFF;
    buffer[10] = status->colorMode;

    return 11;
}

int serialize_last_message(Mgpu_LastMessageResponse *lastMessage, uint8_t buffer[], size_t bufferSize) {
    assert(lastMessage != NULL);

    buffer[0] = Mgpu_Response_LastMessage;

    int bufferIndex = 1;
    while (true) {
        int messageIndex = bufferIndex - 1;
        char character = lastMessage->message[messageIndex];
        if (character == '\0') {
            break;
        }

        if (bufferIndex >= bufferSize) {
            return MGPU_ERROR_BUFFER_TOO_SMALL;
        }

        buffer[bufferIndex] = character;
        bufferIndex++;
    }

    return bufferIndex;
}

int mgpu_serialize_response(Mgpu_Response *response, uint8_t buffer[], size_t bufferSize) {
    assert(response != NULL);
    assert(buffer != NULL);
    assert(bufferSize > 1);

    switch (response->type) {
        case Mgpu_Response_Status:
            return serialize_status(&response->status, buffer, bufferSize);

        case Mgpu_Response_LastMessage:
            return serialize_last_message(&response->lastMessage, buffer, bufferSize);

        default:
            return MGPU_ERROR_UNKNOWN_RESPONSE_TYPE;
    }
}
