#include <stdio.h>
#include <string.h>
#include "microgpu-common/common.h"
#include "textures.h"
#include "microgpu-common/messages.h"

void mgpu_exec_texture_define(Mgpu_TextureManager *textureManager, Mgpu_DefineTextureOperation *operation) {
    assert(textureManager != NULL);
    assert(operation != NULL);

    if (operation->textureId == 0 || operation->textureId > 200) {
        char *msg = mgpu_message_get_pointer();
        assert(msg != NULL);

        snprintf(msg,
                 MESSAGE_MAX_LEN,
                 "Cannot define texture id %u, as its reserved for internal usage",
                 operation->textureId);

        return;
    }

    Mgpu_TextureDefinition info = {
            .id = operation->textureId,
            .width = operation->width,
            .height = operation->height,
            .transparentColor = operation->transparentColor,
            .flags = MGPU_TEXTURE_USE_SLOW_RAM,
    };

    mgpu_texture_define(textureManager, &info, 1);
}

void mgpu_exec_texture_append(Mgpu_TextureManager *textureManager, Mgpu_AppendTexturePixelOperation *operation) {
    assert(textureManager != NULL);
    assert(operation != NULL);
    assert(operation->pixelBytes != NULL);

    Mgpu_Texture *texture = mgpu_texture_get(textureManager, operation->textureId);
    if (texture == NULL) {
        char *msg = mgpu_message_get_pointer();
        assert(msg != NULL);

        snprintf(msg,
                 MESSAGE_MAX_LEN,
                 "Append to texture %u failed: texture not defined",
                 operation->textureId);

        return;
    }

    size_t pixelsLeft = (texture->width * texture->height) - texture->pixelsWritten;
    size_t pixelsToWrite = min(pixelsLeft, operation->pixelCount);

    Mgpu_Color *pixel = texture->pixels + texture->pixelsWritten;
    const uint8_t *byte = operation->pixelBytes;
    for (int x = 0; x < pixelsToWrite; x++) {
        size_t nextByteIndex;
        Mgpu_Color color = mgpu_color_deserialize(byte, 0, &nextByteIndex);
        *pixel = color;

        byte += nextByteIndex;
        pixel++;
    }

    texture->pixelsWritten += pixelsToWrite;
}

void mgpu_exec_texture_draw(Mgpu_TextureManager *textureManager, Mgpu_DrawTextureOperation *operation) {
    assert(textureManager != NULL);
    assert(operation != NULL);

    if (operation->sourceWidth == 0 || operation->sourceHeight == 0) {
        // nothing to draw
        return;
    }

    Mgpu_Texture *sourceTexture = mgpu_texture_get(textureManager, operation->sourceTextureId);
    if (sourceTexture == NULL) {
        char *msg = mgpu_message_get_pointer();
        assert(msg != NULL);

        snprintf(msg,
                 MESSAGE_MAX_LEN,
                 "Attempted to draw from source texture id %u, but that texture is not defined",
                 operation->sourceTextureId);

        return;
    }

    Mgpu_Texture *targetTexture = mgpu_texture_get(textureManager, operation->targetTextureId);
    if (targetTexture == NULL) {
        char *msg = mgpu_message_get_pointer();
        assert(msg != NULL);

        snprintf(msg,
                 MESSAGE_MAX_LEN,
                 "Texture draw error: Attempted to draw to target texture id %u, but that texture is not defined",
                 operation->targetTextureId);

        return;
    }

    if (operation->sourceWidth + operation->sourceStartX > sourceTexture->width) {
        char *msg = mgpu_message_get_pointer();
        assert(msg != NULL);

        strncpy(msg, "Texture draw error: Drawing more horizontal pixels than source texture contains",
                MESSAGE_MAX_LEN);
        return;
    }

    if (operation->sourceHeight + operation->sourceStartY > sourceTexture->height) {
        char *msg = mgpu_message_get_pointer();
        assert(msg != NULL);

        strncpy(msg, "Texture draw error: Drawing more horizontal pixels than source texture contains",
                MESSAGE_MAX_LEN);
        return;
    }

    int startX = max(operation->targetStartX, 0);
    int startY = max(operation->targetStartY, 0);
    int lastX = min(operation->targetStartX + operation->sourceWidth, targetTexture->width - 1);
    int lastY = min(operation->targetStartY + operation->sourceHeight, targetTexture->height - 1);
    int width = lastX - startX;
    int height = lastY - startY;

    if (startX >= targetTexture->width ||
        startY >= targetTexture->height ||
        width == 0 ||
        height == 0) {
        return;
    }

    size_t sourceOffset = ((startY - operation->targetStartY + operation->sourceStartY) * sourceTexture->width) +
                          (startX - operation->targetStartX + operation->sourceStartX);

    size_t targetOffset = (startY * targetTexture->width) + startX;

    Mgpu_Color *sourceRowStart = sourceTexture->pixels + sourceOffset;
    Mgpu_Color *targetRowStart = targetTexture->pixels + targetOffset;

    for (int row = 0; row < height; row++) {
        Mgpu_Color *source = sourceRowStart;
        Mgpu_Color *target = targetRowStart;

        if (operation->ignoreTransparency) {
            memcpy(target, source, width * sizeof(Mgpu_Color));
        } else {
            for (int col = 0; col < width; col++) {
                if (*source != sourceTexture->transparencyColor) {
                    *target = *source;
                }

                target++;
                source++;
            }
        }

        sourceRowStart += sourceTexture->width;
        targetRowStart += targetTexture->width;
    }
}
