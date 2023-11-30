#pragma once

#include "microgpu-common/operations/operations.h"
#include "microgpu-common/texture_manager.h"

void mgpu_exec_font_draw(Mgpu_TextureManager *textureManager, Mgpu_DrawCharsOperation *operation);

