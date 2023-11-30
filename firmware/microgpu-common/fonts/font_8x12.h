#pragma once

#include "microgpu-common/colors/color.h"
#include "microgpu-common/texture_manager.h"

void mgpu_font_8x12_write(Mgpu_Texture *texture,
                          char *text,
                          Mgpu_Color color,
                          uint16_t startX,
                          uint16_t startY);
