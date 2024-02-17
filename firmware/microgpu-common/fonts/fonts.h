#pragma once

#include <stdint.h>
#include "microgpu-common/texture_manager.h"

typedef enum {
    Mgpu_Font_Unspecified = 0,
    // Mgpu_Font_Font4x6 = 1,
    // Mgpu_Font_Font4x8 = 2,
    // Mgpu_Font_Font6x8 = 3,
    // Mgpu_Font_Font8x8 = 4,
    Mgpu_Font_Font8x12 = 5,
    // Mgpu_Font_Font8x16 = 6,
    Mgpu_Font_Font12x16 = 7,
    // Mgpu_Font_Font12x20 = 8,
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