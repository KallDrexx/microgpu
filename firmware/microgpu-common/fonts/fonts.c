#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "microgpu-common/messages.h"
#include "fonts.h"
#include "font_8x12.h"
#include "font_12x16.h"

void mgpu_font_draw(Mgpu_TextureManager *textureManager,
                    Mgpu_FontId fontId,
                    uint8_t destinationTextureId,
                    char *text,
                    Mgpu_Color color,
                    uint16_t startX,
                    uint16_t startY) {
    assert(textureManager != NULL);

    if (text == NULL || strlen(text) == 0) {
        return; // nothing to draw
    }

    Mgpu_Texture *texture = mgpu_texture_get(textureManager, destinationTextureId);
    if (texture == NULL) {
        char *message = mgpu_message_get_pointer();
        assert(message != NULL);

        snprintf(message, MESSAGE_MAX_LEN, "Font draw failed: destination texture id %u does not exist",
                 destinationTextureId);
        return;
    }

    if (startX >= texture->width || startY >= texture->height) {
        return;
    }

    switch (fontId) {
        case Mgpu_Font_Font8x12:
            mgpu_font_8x12_write(texture, text, color, startX, startY);
            break;

        case Mgpu_Font_Font12x16:
            mgpu_font_12x16_write(texture, text, color, startX, startY);
            break;

        default: {
            char *message = mgpu_message_get_pointer();
            assert(message != NULL);

            snprintf(message, MESSAGE_MAX_LEN, "Invalid font id specified of %u", fontId);
            break;
        }
    }
}
