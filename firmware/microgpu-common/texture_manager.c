#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "messages.h"
#include "texture_manager.h"

#define NUM_TEXTURES 255

struct Mgpu_TextureManager {
    const Mgpu_Allocator *allocator;
    Mgpu_Texture **textures;
};

void free_texture(Mgpu_Texture *texture, const Mgpu_Allocator *allocator) {
    assert(texture != NULL);
    assert(allocator != NULL);

    allocator->FreeFn(texture);
}

Mgpu_TextureManager *mgpu_texture_manager_new(const Mgpu_Allocator *allocator) {
    assert(allocator != NULL);

    Mgpu_TextureManager *manager = allocator->AllocateFn(sizeof(Mgpu_TextureManager));
    if (manager == NULL) {
        char *message = mgpu_message_get_pointer();
        assert(message != NULL);

        strncpy(message, "Failed to allocate texture manager", MESSAGE_MAX_LEN);

        return NULL;
    }

    manager->textures = allocator->AllocateFn(sizeof(Mgpu_Texture *) * NUM_TEXTURES);
    if (manager->textures == NULL) {
        char *message = mgpu_message_get_pointer();
        assert(message != NULL);

        strncpy(message, "Failed to allocate texture manager array", MESSAGE_MAX_LEN);
        mgpu_texture_manager_free(manager);

        return NULL;
    }

    manager->allocator = allocator;
    memset(manager->textures, 0, sizeof(Mgpu_Texture *) * NUM_TEXTURES);

    return manager;
}

void mgpu_texture_manager_free(Mgpu_TextureManager *textureManager) {
    if (textureManager != NULL) {
        if (textureManager->textures != NULL) {
            for (int x = 0; x < NUM_TEXTURES; x++) {
                if (textureManager->textures[x] != NULL) {
                    free_texture(textureManager->textures[x], textureManager->allocator);
                    textureManager->textures[x] = NULL;
                }
            }

            textureManager->allocator->FreeFn(textureManager->textures);
            textureManager->textures = NULL;
        }

        textureManager->allocator->FreeFn(textureManager);
    }
}

bool mgpu_texture_define(Mgpu_TextureManager *textureManager, Mgpu_TextureDefinition *info, uint8_t scale) {
    assert(textureManager != NULL);
    assert(textureManager->textures != NULL);
    assert(info != NULL);
    assert(scale > 0);

    if (info->id >= NUM_TEXTURES) {
        char *msg = mgpu_message_get_pointer();
        assert(msg != NULL);

        snprintf(msg,
                 MESSAGE_MAX_LEN,
                 "Defining texture id %u failed as id larger than set count of %u",
                 info->id, NUM_TEXTURES);

        return false;
    }

    Mgpu_Texture *texture = textureManager->textures[info->id];
    if (texture != NULL) {
        // Texture is being redefined
        free_texture(texture, textureManager->allocator);
        textureManager->textures[info->id] = NULL;
    }

    uint16_t width = info->width / scale;
    uint16_t height = info->height / scale;

    size_t pixelCount = width * height;
    if (pixelCount > 0) {
        texture = textureManager->allocator->AllocateFn(sizeof(Mgpu_Texture) + pixelCount * sizeof(Mgpu_Color));
        if (texture == NULL) {
            char *msg = mgpu_message_get_pointer();
            snprintf(msg,
                     MESSAGE_MAX_LEN,
                     "Defining texture id %u failed: could not allocate texture space",
                     info->id);

            return false;
        }

        texture->height = height;
        texture->width = width;
        texture->transparencyColor = info->transparentColor;
        texture->pixelsWritten = 0;
        texture->scale = scale;

        Mgpu_Color color = mgpu_color_from_rgb888(0, 0, 0);
        memset(texture->pixels, color, width * height * sizeof(Mgpu_Color));
        textureManager->textures[info->id] = texture;
    }

    return true;
}

Mgpu_Texture *mgpu_texture_get(Mgpu_TextureManager *textureManager, uint8_t id) {
    assert(textureManager != NULL);
    assert(textureManager->textures != NULL);

    if (id >= NUM_TEXTURES) {
        char *msg = mgpu_message_get_pointer();
        assert(msg != NULL);
        snprintf(msg,
                 MESSAGE_MAX_LEN,
                 "Can't get texture id %u, as that id is larger than the texture count of %u",
                 id, NUM_TEXTURES);

        return NULL;
    }

    return textureManager->textures[id];
}
