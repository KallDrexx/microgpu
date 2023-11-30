#include <assert.h>
#include <stdio.h>
#include "microgpu-common/messages.h"
#include "microgpu-common/colors/color.h"
#include "operation_deserializer.h"

bool deserialize_status_op(Mgpu_Operation *operation) {
    operation->type = Mgpu_Operation_GetStatus;
    return true;
}

bool deserialize_last_message_op(Mgpu_Operation *operation) {
    operation->type = Mgpu_Operation_GetLastMessage;
    return true;
}

bool deserialize_initialize_op(const uint8_t bytes[], size_t size, Mgpu_Operation *operation) {
    if (size < 2) {
        return false;
    }

    operation->type = Mgpu_Operation_Initialize;
    operation->initialize.frameBufferScale = bytes[1];

    return true;
}

bool deserialize_draw_rectangle(const uint8_t bytes[], size_t size, Mgpu_Operation *operation) {
    if (size < 10 + mgpu_color_bytes_per_pixel()) {
        return false;
    }

    operation->type = Mgpu_Operation_DrawRectangle;
    operation->drawRectangle.textureId = bytes[1];
    operation->drawRectangle.startX = ((uint16_t) bytes[2] << 8) | bytes[3];
    operation->drawRectangle.startY = ((uint16_t) bytes[4] << 8) | bytes[5];
    operation->drawRectangle.width = ((uint16_t) bytes[6] << 8) | bytes[7];
    operation->drawRectangle.height = ((uint16_t) bytes[8] << 8) | bytes[9];

    size_t nextByteIndex;
    operation->drawRectangle.color = mgpu_color_deserialize(bytes, 10, &nextByteIndex);

    return true;
}

bool deserialize_draw_triangle(const uint8_t bytes[], size_t size, Mgpu_Operation *operation) {
    if (size < 14 + mgpu_color_bytes_per_pixel()) {
        return false;
    }

    operation->type = Mgpu_Operation_DrawTriangle;
    operation->drawTriangle.textureId = bytes[1];
    operation->drawTriangle.x0 = ((uint16_t) bytes[2] << 8) | bytes[3];
    operation->drawTriangle.y0 = ((uint16_t) bytes[4] << 8) | bytes[5];
    operation->drawTriangle.x1 = ((uint16_t) bytes[6] << 8) | bytes[7];
    operation->drawTriangle.y1 = ((uint16_t) bytes[8] << 8) | bytes[9];
    operation->drawTriangle.x2 = ((uint16_t) bytes[10] << 8) | bytes[11];
    operation->drawTriangle.y2 = ((uint16_t) bytes[12] << 8) | bytes[13];

    size_t nextByteIndex;
    operation->drawTriangle.color = mgpu_color_deserialize(bytes, 14, &nextByteIndex);

    return true;
}

bool deserialize_present_framebuffer(Mgpu_Operation *operation) {
    operation->type = Mgpu_Operation_PresentFramebuffer;
    return true;
}

bool deserialize_reset(const uint8_t bytes[], size_t size, Mgpu_Operation *operation) {
    // Require magic bytes just to ensure the reset request was valid and wasn't caused
    // by an incorrect read.
    if (size < 4 || bytes[1] != 0x09 || bytes[2] != 0x13 || bytes[3] != 0xac) {
        return false;
    }

    operation->type = Mgpu_Operation_Reset;
    return true;
}

bool deserialize_batch(const uint8_t bytes[], size_t size, Mgpu_Operation *operation) {
    if (size < 3) {
        char *msg = mgpu_message_get_pointer();
        assert(msg != NULL);

        snprintf(msg, MESSAGE_MAX_LEN, "Batch message had too few characters (size %zu)", size);
        return false;
    }

    uint16_t innerSize = (bytes[1] << 8) | bytes[2];
    if (innerSize > size - 3) {
        char *msg = mgpu_message_get_pointer();
        assert(msg != NULL);

        snprintf(msg, MESSAGE_MAX_LEN, "Batch message inner size too large (size %zu, innerSize %u)", size, innerSize);
        return false;
    }

    operation->type = Mgpu_Operation_Batch;
    operation->batchOperation.byteLength = innerSize;

    // This should be ok as the operation should not be used by the time
    // the next databus operation occurs.
    operation->batchOperation.bytes = bytes + 3;

    return true;
}

bool deserialize_define_texture(const uint8_t bytes[], size_t size, Mgpu_Operation *operation) {
    if (size < 6 + mgpu_color_bytes_per_pixel()) {
        return false;
    }

    operation->type = Mgpu_Operation_DefineTexture;
    operation->defineTexture.textureId = bytes[1];
    operation->defineTexture.width = ((uint16_t) bytes[2] << 8) | bytes[3];
    operation->defineTexture.height = ((uint16_t) bytes[4] << 8) | bytes[5];

    size_t nextByteIndex;
    operation->defineTexture.transparentColor = mgpu_color_deserialize(bytes, 6, &nextByteIndex);

    return true;
}

bool deserialize_append_pixels(const uint8_t bytes[], size_t size, Mgpu_Operation *operation) {
    if (size < 4) {
        return false;
    }

    operation->type = Mgpu_Operation_AppendTexturePixels;
    operation->appendTexturePixels.textureId = bytes[1];
    operation->appendTexturePixels.pixelCount = ((uint16_t) bytes[2] << 8) | bytes[3];

    // Make sure the size is within bounds of the byte array
    size_t bytesNeededForPixels = mgpu_color_bytes_per_pixel() * operation->appendTexturePixels.pixelCount;
    if (bytesNeededForPixels > size - 4) {
        char *msg = mgpu_message_get_pointer();
        assert(msg != NULL);

        snprintf(msg,
                 MESSAGE_MAX_LEN,
                 "Append to texture op had a pixel size of %u, but only %u bytes were provided",
                 operation->appendTexturePixels.pixelCount,
                 (int) (size - 4));

        return false;
    }

    // This should be ok as the operation should not be used by the time
    // the next databus operation occurs.
    operation->appendTexturePixels.pixelBytes = (bytes + 4);

    return true;
}

bool deserialize_draw_texture(const uint8_t bytes[], size_t size, Mgpu_Operation *operation) {
    if (size < 16) {
        return false;
    }

    operation->type = Mgpu_Operation_DrawTexture;
    operation->drawTexture.sourceTextureId = bytes[1];
    operation->drawTexture.targetTextureId = bytes[2];
    operation->drawTexture.sourceStartX = ((uint16_t) bytes[3] << 8) | bytes[4];
    operation->drawTexture.sourceStartY = ((uint16_t) bytes[5] << 8) | bytes[6];
    operation->drawTexture.sourceWidth = ((uint16_t) bytes[7] << 8) | bytes[8];
    operation->drawTexture.sourceHeight = ((uint16_t) bytes[9] << 8) | bytes[10];

    // NOTE: This assumes all calling systems use the same negative number representation
    // (2's compliment??) as the GPU's architecture.
    operation->drawTexture.targetStartX = (int16_t) (((int16_t) bytes[11] << 8) | bytes[12]);
    operation->drawTexture.targetStartY = (int16_t) (((int16_t) bytes[13] << 8) | bytes[14]);

    // Flags
    operation->drawTexture.ignoreTransparency = bytes[15] & 0x01;

    return true;
}

bool deserialize_draw_chars(const uint8_t bytes[], size_t size, Mgpu_Operation *operation) {
    if (size < 6 + mgpu_color_bytes_per_pixel()) {
        return false;
    }

    operation->type = Mgpu_Operation_DrawChars;
    operation->drawChars.fontId = bytes[1];
    operation->drawChars.textureId = bytes[2];

    size_t nextByteIndex;
    operation->drawChars.color = mgpu_color_deserialize(bytes, 3, &nextByteIndex);
    operation->drawChars.startX = ((uint16_t) bytes[nextByteIndex + 0] << 8) | bytes[nextByteIndex + 1];
    operation->drawChars.startY = ((uint16_t) bytes[nextByteIndex + 2] << 8) | bytes[nextByteIndex + 3];
    operation->drawChars.numCharacters = bytes[nextByteIndex + 4];
    operation->drawChars.characters = bytes + nextByteIndex + 5;

    return true;
}

bool mgpu_operation_deserialize(const uint8_t bytes[], size_t size, Mgpu_Operation *operation) {
    assert(bytes != NULL);
    assert(operation != NULL);

    if (size == 0) {
        // Need at least one byte
        return false;
    }

    switch (bytes[0]) {
        case Mgpu_Operation_Initialize:
            return deserialize_initialize_op(bytes, size, operation);

        case Mgpu_Operation_GetStatus:
            return deserialize_status_op(operation);

        case Mgpu_Operation_GetLastMessage:
            return deserialize_last_message_op(operation);

        case Mgpu_Operation_DrawRectangle:
            return deserialize_draw_rectangle(bytes, size, operation);

        case Mgpu_Operation_DrawTriangle:
            return deserialize_draw_triangle(bytes, size, operation);

        case Mgpu_Operation_PresentFramebuffer:
            return deserialize_present_framebuffer(operation);

        case Mgpu_Operation_Batch:
            return deserialize_batch(bytes, size, operation);

        case Mgpu_Operation_Reset:
            return deserialize_reset(bytes, size, operation);

        case Mgpu_Operation_DefineTexture:
            return deserialize_define_texture(bytes, size, operation);

        case Mgpu_Operation_AppendTexturePixels:
            return deserialize_append_pixels(bytes, size, operation);

        case Mgpu_Operation_DrawTexture:
            return deserialize_draw_texture(bytes, size, operation);

        case Mgpu_Operation_DrawChars:
            return deserialize_draw_chars(bytes, size, operation);

        default: {
            char *msg = mgpu_message_get_pointer();
            assert(msg != NULL);

            snprintf(msg, MESSAGE_MAX_LEN, "Operation id %u is not a known operation id", bytes[0]);
            return false;
        }
    }
}
