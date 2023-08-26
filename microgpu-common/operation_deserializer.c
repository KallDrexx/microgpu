#include <assert.h>
#include "color.h"
#include "operation_deserializer.h"

Mgpu_Color deserialize_color(const uint8_t bytes[], size_t firstColorByteIndex);

#ifdef MGPU_COLOR_MODE_USE_RGB565

Mgpu_Color deserialize_color(const uint8_t bytes[], size_t firstColorByteIndex) {
    uint8_t red = bytes[firstColorByteIndex] & 0xF8;
    uint8_t green = (bytes[firstColorByteIndex] & 0x07) << 3 | (bytes[firstColorByteIndex + 1] & 0xE0);
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

        default:
            return false;
    }
}
