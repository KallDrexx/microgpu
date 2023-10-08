#include <assert.h>
#include <stdio.h>
#include "microgpu-common/messages.h"
#include "color.h"
#include "operation_deserializer.h"

Mgpu_Color deserialize_color(const uint8_t bytes[], size_t firstColorByteIndex);

#ifdef MGPU_COLOR_MODE_USE_RGB565

Mgpu_Color deserialize_color(const uint8_t bytes[], size_t firstColorByteIndex) {
    uint8_t red = (bytes[firstColorByteIndex] & 0xF8) >> 3;
    uint8_t green = (bytes[firstColorByteIndex] & 0x07) << 3 | (bytes[firstColorByteIndex + 1] & 0xE0) >> 5;
    uint8_t blue = bytes[firstColorByteIndex + 1] & 0x1F;

    return mgpu_color_from_rgb565(red, green, blue);
}

#else
#error "No color mode specified"
#endif

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
    if (size < sizeof(Mgpu_DrawRectangleOperation) + 1) {
        return false;
    }

    operation->type = Mgpu_Operation_DrawRectangle;
    operation->drawRectangle.startX = ((uint16_t) bytes[1] << 8) | bytes[2];
    operation->drawRectangle.startY = ((uint16_t) bytes[3] << 8) | bytes[4];
    operation->drawRectangle.width = ((uint16_t) bytes[5] << 8) | bytes[6];
    operation->drawRectangle.height = ((uint16_t) bytes[7] << 8) | bytes[8];
    operation->drawRectangle.color = deserialize_color(bytes, 9);

    return true;
}

bool deserialize_draw_triangle(const uint8_t bytes[], size_t size, Mgpu_Operation *operation) {
    if (size < sizeof(Mgpu_DrawTriangleOperation) + 1) {
        return false;
    }

    operation->type = Mgpu_Operation_DrawTriangle;
    operation->drawTriangle.x0 = ((uint16_t) bytes[1] << 8) | bytes[2];
    operation->drawTriangle.y0 = ((uint16_t) bytes[3] << 8) | bytes[4];
    operation->drawTriangle.x1 = ((uint16_t) bytes[5] << 8) | bytes[6];
    operation->drawTriangle.y1 = ((uint16_t) bytes[7] << 8) | bytes[8];
    operation->drawTriangle.x2 = ((uint16_t) bytes[9] << 8) | bytes[10];
    operation->drawTriangle.y2 = ((uint16_t) bytes[11] << 8) | bytes[12];
    operation->drawTriangle.color = deserialize_color(bytes, 13);

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
        char msg[MESSAGE_MAX_LEN] = {0};
        snprintf(msg, MESSAGE_MAX_LEN, "Batch message had too few characters (size %zu)", size);
        mgpu_message_set(msg);
        return false;
    }

    uint16_t innerSize = (bytes[1] << 8) | bytes[2];
    if (innerSize > size - 3) {
        char msg[MESSAGE_MAX_LEN] = {0};
        snprintf(msg, MESSAGE_MAX_LEN, "Batch message inner size too large (size %zu, innerSize %u)", size, innerSize);
        mgpu_message_set(msg);
        return false;
    }

    operation->type = Mgpu_Operation_Batch;
    operation->batchOperation.byteLength = innerSize;

    // This should be ok as the operation should not be used by the time
    // the next databus operation occurs.
    operation->batchOperation.bytes = bytes + 3;

    return true;
}

bool deserialize_set_texture_count(const uint8_t bytes[], size_t size, Mgpu_Operation *operation) {
    if (size < sizeof(Mgpu_SetTextureCountOperation) + 1) {
        return false;
    }

    operation->type = Mgpu_Operation_SetTextureCount;
    operation->setTextureCount.textureCount = bytes[1];

    return true;
}

bool deserialize_define_texture(const uint8_t bytes[], size_t size, Mgpu_Operation *operation) {
    if (size < sizeof(Mgpu_DefineTextureOperation) + 1) {
        return false;
    }

    operation->type = Mgpu_Operation_DefineTexture;
    operation->defineTexture.textureId = bytes[1];
    operation->defineTexture.width = ((uint16_t) bytes[1] << 8) | bytes[2];
    operation->defineTexture.height = ((uint16_t) bytes[3] << 8) | bytes[4];
    operation->defineTexture.transparentColor = deserialize_color(bytes, 5);

    return true;
}

bool deserialize_append_pixels(const uint8_t bytes[], size_t size, Mgpu_Operation *operation) {
    if (size < sizeof(Mgpu_AppendTexturePixelOperation) + 1) {
        return false;
    }

    operation->type = Mgpu_Operation_AppendTexturePixels;
    operation->appendTexturePixels.textureId = bytes[1];
    operation->appendTexturePixels.pixelCount = ((uint16_t) bytes[2] << 8) | bytes[3];

    // Make sure the size is within bounds of the byte array
    size_t bytesNeededForPixels = sizeof(Mgpu_Color) * operation->appendTexturePixels.pixelCount;
    if (bytesNeededForPixels > size - 4) {
        char msg[MESSAGE_MAX_LEN] = {0};
        snprintf(msg,
                 MESSAGE_MAX_LEN,
                 "Append to texture op had a pixel size of %u, but only %u bytes were provided",
                 operation->appendTexturePixels.pixelCount,
                 (int) (size - 4));

        mgpu_message_set(msg);
        return false;
    }

    // This should be ok as the operation should not be used by the time
    // the next databus operation occurs.
    operation->appendTexturePixels.pixels = (Mgpu_Color *) (bytes + 4);

    return true;
}

bool deserialize_draw_texture(const uint8_t bytes[], size_t size, Mgpu_Operation *operation) {
    if (size < sizeof(Mgpu_DrawTextureOperation) + 1) {
        return false;
    }

    operation->type = Mgpu_Operation_RenderTexture;
    operation->drawTexture.textureId = bytes[1];

    // NOTE: This assumes all calling systems use the same negative number representation
    // (2's compliment??) as the GPU's architecture.
    operation->drawTexture.xPosition = (int16_t) (((int16_t) bytes[2] >> 8) | bytes[3]);
    operation->drawTexture.yPosition = (int16_t) (((int16_t) bytes[4] >> 8) | bytes[5]);

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

        case Mgpu_Operation_SetTextureCount:
            return deserialize_set_texture_count(bytes, size, operation);

        case Mgpu_Operation_DefineTexture:
            return deserialize_define_texture(bytes, size, operation);

        case Mgpu_Operation_AppendTexturePixels:
            return deserialize_append_pixels(bytes, size, operation);

        case Mgpu_Operation_RenderTexture:
            return deserialize_draw_texture(bytes, size, operation);

        default:
            return false;
    }
}
