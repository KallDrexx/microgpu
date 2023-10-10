#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "messages.h"
#include "texture_manager.h"

typedef struct {
    uint16_t width, height;
    Mgpu_Color transparentColor;
    size_t pixelsWritten;
    Mgpu_Color *pixels;
} Texture;

struct Mgpu_TextureManager {
    const Mgpu_Allocator *allocator;
    uint8_t textureCount;
    Texture *textures;
};

void free_texture(Texture *texture, const Mgpu_Allocator *allocator) {
    assert(texture != NULL);
    assert(allocator != NULL);

    allocator->FreeFn(texture->pixels);
    texture->width = 0;
    texture->height = 0;
    texture->pixels = NULL;
}

void update_texture(Texture *texture, Mgpu_TextureInfo *info, const Mgpu_Allocator *allocator) {
    assert(texture != NULL);
    assert(info != NULL);

    // Even if the height and width are the same, assume an update call is a desire
    // to clear the texture's pixel data. Most likely new pixel data is going to
    // come in.
    texture->width = info->width;
    texture->height = info->height;
    texture->transparentColor = info->transparentColor;
    texture->pixelsWritten = 0;
    allocator->FreeFn(texture->pixels);
    texture->pixels = allocator->AllocateFn(sizeof(Mgpu_Color) * info->height * info->width);

    if (texture->pixels == NULL) {
        // We could not allocate new pixel data, so this texture is now invalid.
        free_texture(texture, allocator);

        char msg[256];
        snprintf(msg,
                 sizeof(msg),
                 "Defining texture id %u failed: failed to allocate pixel space",
                 info->id);
        mgpu_message_set(msg);
    }
}

Mgpu_TextureManager *mgpu_texture_manager_new(const Mgpu_Allocator *allocator) {
    assert(allocator != NULL);

    Mgpu_TextureManager *manager = allocator->AllocateFn(sizeof(Mgpu_TextureManager));
    if (manager == NULL) {
        mgpu_message_set("Failed to allocate texture manager");
        return NULL;
    }

    manager->allocator = allocator;
    manager->textureCount = 0;
    manager->textures = NULL;

    return manager;
}

void mgpu_texture_manager_free(Mgpu_TextureManager *textureManager) {
    if (textureManager != NULL) {
        if (textureManager->textures != NULL) {
            for (int x = 0; x < textureManager->textureCount; x++) {
                free_texture(&textureManager->textures[x], textureManager->allocator);
            }

            textureManager->allocator->FreeFn(textureManager->textures);
            textureManager->textures = NULL;
            textureManager->textureCount = 0;
        }

        textureManager->allocator->FreeFn(textureManager);
    }
}

void mgpu_texture_set_count(Mgpu_TextureManager *textureManager, uint8_t count) {
    assert(textureManager != NULL);

    if (textureManager->textureCount > 0) {
        for (int x = 0; x < textureManager->textureCount; x++) {
            free_texture(&textureManager->textures[x], textureManager->allocator);
        }

        textureManager->allocator->FreeFn(textureManager->textures);
        textureManager->textures = NULL;
        textureManager->textureCount = 0;
    }

    if (count > 0) {
        textureManager->textures = textureManager->allocator->AllocateFn(sizeof(Texture) * count);
        if (textureManager->textures == NULL) {
            char msg[256];
            snprintf(msg,
                     sizeof(msg),
                     "Could not set texture count to %u as texture allocation failed",
                     count);
            mgpu_message_set(msg);

            return;
        }

        textureManager->textureCount = count;
        for (int x = 0; x < count; x++) {
            textureManager->textures[x].width = 0;
            textureManager->textures[x].height = 0;
            textureManager->textures[x].pixelsWritten = 0;
            textureManager->textures[x].pixels = NULL;
        }
    }
}

void mgpu_texture_define(Mgpu_TextureManager *textureManager, Mgpu_TextureInfo *info) {
    assert(textureManager != NULL);
    assert(info != NULL);

    if (info->id >= textureManager->textureCount) {
        char msg[256];
        snprintf(msg,
                 sizeof(msg),
                 "Defining texture id %u failed as id larger than set count of %u",
                 info->id, textureManager->textureCount);
        mgpu_message_set(msg);
        return;
    }

    Texture *texture = &textureManager->textures[info->id];
    if (info->width > 0 && info->height > 0) {
        update_texture(texture, info, textureManager->allocator);
    } else {
        free_texture(texture, textureManager->allocator);
    }
}

void mgpu_texture_append(Mgpu_TextureManager *textureManager, uint8_t id, uint16_t pixelCount, Mgpu_Color *pixels) {
    assert(textureManager != NULL);
    assert(pixels != NULL);

    if (id >= textureManager->textureCount) {
        char msg[256];
        snprintf(msg,
                 sizeof(msg),
                 "Can't add pixels to texture id %u, as that id is larger than the texture count of %u",
                 id, textureManager->textureCount);
        mgpu_message_set(msg);
        return;
    }

    Texture *texture = &textureManager->textures[id];
    size_t pixelsLeft = (texture->width * texture->height) - texture->pixelsWritten;
    size_t pixelsToWrite = min(pixelsLeft, pixelCount);
    if (pixelsToWrite == 0) {
        return;
    }

    Mgpu_Color *startPos = texture->pixels + texture->pixelsWritten;
    memcpy(startPos, pixels, pixelsToWrite * sizeof(Mgpu_Color));
    texture->pixelsWritten += pixelsToWrite;
}

void mgpu_texture_draw(Mgpu_TextureManager *textureManager,
                       Mgpu_FrameBuffer *frameBuffer,
                       uint16_t textureId,
                       int16_t xPosition,
                       int16_t yPosition) {
    assert(textureManager != NULL);
    assert(frameBuffer != NULL);
    assert(frameBuffer->pixels != NULL);

    if (textureId >= textureManager->textureCount) {
        return;
    }

    Texture *texture = &textureManager->textures[textureId];

    uint16_t startX = max(xPosition, 0);
    uint16_t startY = max(yPosition, 0);
    uint16_t lastX = min(xPosition + texture->width, frameBuffer->width - 1);
    uint16_t lastY = min(yPosition + texture->height, frameBuffer->height - 1);
    uint16_t width = lastX - startX;
    uint16_t height = lastY - startY;

    size_t sourceOffset = ((startY - yPosition) * texture->width) + (startX - xPosition);
    size_t targetOffset = (startY * frameBuffer->width) + startX;

    Mgpu_Color *sourceRowStart = texture->pixels + sourceOffset;
    Mgpu_Color *targetRowStart = frameBuffer->pixels + targetOffset;

    for (int row = 0; row < height; row++) {
        Mgpu_Color *source = sourceRowStart;
        Mgpu_Color *target = targetRowStart;

        for (int col = 0; col < width; col++) {
            if (*source != texture->transparentColor) {
                *target = *source;
            }

            target++;
            source++;
        }

        sourceRowStart += texture->width;
        targetRowStart += frameBuffer->width;
    }
}
