#include "textures.h"

void mgpu_exec_texture_count(Mgpu_TextureManager *textureManager, Mgpu_SetTextureCountOperation *operation) {
    assert(textureManager != NULL);
    assert(operation != NULL);

    mgpu_texture_set_count(textureManager, operation->textureCount);
}

void mgpu_exec_texture_define(Mgpu_TextureManager *textureManager, Mgpu_DefineTextureOperation *operation) {
    assert(textureManager != NULL);
    assert(operation != NULL);

    Mgpu_TextureInfo info = {
            .id = operation->textureId,
            .width = operation->width,
            .height = operation->height,
            .transparentColor = operation->transparentColor,
    };

    mgpu_texture_define(textureManager, &info);
}

void mgpu_exec_texture_append(Mgpu_TextureManager *textureManager, Mgpu_AppendTexturePixelOperation *operation) {
    assert(textureManager != NULL);
    assert(operation != NULL);

    mgpu_texture_append(textureManager, operation->textureId, operation->pixelCount, operation->pixelBytes);
}

void mgpu_exec_texture_render(Mgpu_TextureManager *textureManager,
                              Mgpu_FrameBuffer *frameBuffer,
                              Mgpu_DrawTextureOperation *operation) {
    assert(textureManager != NULL);
    assert(operation != NULL);

    mgpu_texture_draw(textureManager, frameBuffer, operation->textureId, operation->xPosition, operation->yPosition);
}
