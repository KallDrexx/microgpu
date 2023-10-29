#pragma once

#include "microgpu-common/operations.h"
#include "microgpu-common/texture_manager.h"

void mgpu_exec_texture_define(Mgpu_TextureManager *textureManager, Mgpu_DefineTextureOperation *operation);

void mgpu_exec_texture_append(Mgpu_TextureManager *textureManager, Mgpu_AppendTexturePixelOperation *operation);

void mgpu_exec_texture_draw(Mgpu_TextureManager *textureManager, Mgpu_DrawTextureOperation *operation);