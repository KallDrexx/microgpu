#pragma once

#include <stdint.h>
#include "microgpu-common/texture_manager.h"

typedef enum {
    Mgpu_Font_Unspecified = 0,
    Mgpu_Font_Font8x12 = 1,
} Mgpu_FontId;

/*
 * Draws1 the specified text in the specified font
 */
void mgpu_font_draw(Mgpu_TextureManager *textureManager,
                    Mgpu_FontId fontId,
                     uint8_t destinationTextureId,
                     char *text,
                     Mgpu_Color color,
                     uint16_t startX,
                     uint16_t startY);