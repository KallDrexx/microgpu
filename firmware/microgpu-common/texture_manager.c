#include <assert.h>
#include <stdio.h>
#include "messages.h"
#include "texture_manager.h"

// number of textures that can be defined
#define NUM_TEXTURES 10

typedef struct {
    uint16_t width, height;
    size_t pixelCount;
    size_t pixelsWritten;
    Mgpu_Color *pixels;
} Texture;

struct Mgpu_TextureManager {
    Mgpu_Allocator *allocator;
    Texture *textures;
};

void clear_texture(Texture *texture, Mgpu_Allocator *allocator) {
    assert(texture != NULL);
    assert(allocator != NULL);

    allocator->FreeFn(texture->pixels);
    texture->width = 0;
    texture->height = 0;
    texture->pixelCount = 0;
    texture->pixels = NULL;
}

Mgpu_TextureManager *mgpu_texture_manager_new(Mgpu_Allocator *allocator) {
    assert(allocator != NULL);

    Mgpu_TextureManager *manager = allocator->AllocateFn(sizeof(Mgpu_TextureManager));
    if (manager == NULL) {
        mgpu_message_set("Failed to allocate texture manager");
        return NULL;
    }

    manager->textures = allocator->AllocateFn(sizeof(Texture) * NUM_TEXTURES);
    if (manager->textures == NULL) {
        mgpu_message_set("Failed to allocate textures for texture manager");
        mgpu_texture_manager_free(manager);
        return NULL;
    }

    for (int x = 0; x < NUM_TEXTURES; x++) {
        Texture *texture = &manager->textures[x];
        texture->pixels = NULL;
        clear_texture(texture, manager->allocator);
    }

    return manager;
}

void mgpu_texture_manager_free(Mgpu_TextureManager *textureManager) {
    if (textureManager != NULL) {
        if (textureManager->textures != NULL) {
            for (int x = 0; x < NUM_TEXTURES; x++) {
                clear_texture(&textureManager->textures[x], textureManager->allocator);
            }

            textureManager->allocator->FreeFn(textureManager->textures);
            textureManager->textures = NULL;
        }

        textureManager->allocator->FreeFn(textureManager);
    }
}

void mgpu_texture_define(Mgpu_TextureManager *textureManager, uint8_t id, uint16_t width, uint16_t height) {
    assert(textureManager != NULL);

    if (id >= NUM_TEXTURES) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Defining texture id %u failed: id too large (max %u)", id, NUM_TEXTURES - 1);
        mgpu_message_set(msg);

        return;
    }

    Texture *texture = &textureManager->textures[id];
    clear_texture(texture, textureManager->allocator);

    texture->pixels = textureManager->allocator->AllocateFn(sizeof(Mgpu_Color) * width * height);
    if (texture->pixels != NULL) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Defining texture id %u failed: failed to allocate pixel data", id);
        mgpu_message_set(msg);

        return;
    }

    texture->width = width;
    texture->height = height;
    texture->pixelCount = width * height;
}

void mgpu_texture_append(Mgpu_TextureManager *textureManager, uint8_t id, uint16_t pixelCount, Mgpu_Color *pixels) {
    assert(textureManager != NULL);

    if (id >= NUM_TEXTURES) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Appending pixel data to texture id %u failed: id too large (max %u)", id, NUM_TEXTURES - 1);
        mgpu_message_set(msg);

        return;
    }


}




