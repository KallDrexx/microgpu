#include <stdio.h>
#include "microgpu-common/common.h"
#include "microgpu-common/messages.h"
#include "rectangle.h"

void mgpu_draw_rectangle(Mgpu_DrawRectangleOperation *drawRectangle, Mgpu_TextureManager *textureManager) {
    assert(textureManager != NULL);

    Mgpu_Texture *texture = mgpu_texture_get(textureManager, drawRectangle->textureId);
    if (texture == NULL) {
        char *msg = mgpu_message_get_pointer();
        assert(msg != NULL);
        snprintf(msg,
                 MESSAGE_MAX_LEN,
                 "Failed to draw rectangle: Target texture with id of %u is not defined",
                 drawRectangle->textureId);

        return;
    }

    if (drawRectangle->startX >= texture->width || drawRectangle->startY >= texture->height) {
        // It starts out of bounds, so nothing to draw.
        return;
    }

    uint16_t endX = min(drawRectangle->startX + drawRectangle->width, texture->width);
    uint16_t endY = min(drawRectangle->startY + drawRectangle->height, texture->height);
    uint16_t adjustedWidth = endX - drawRectangle->startX;
    uint16_t adjustedHeight = endY - drawRectangle->startY;

    Mgpu_Color *pixel = texture->pixels + ((drawRectangle->startY * texture->width) + drawRectangle->startX);
    for (uint16_t row = 0; row < adjustedHeight; row++) {
        for (uint16_t col = 0; col < adjustedWidth; col++) {
            *pixel = drawRectangle->color;
            pixel++;
        }

        pixel += texture->width - adjustedWidth;
    }
}
