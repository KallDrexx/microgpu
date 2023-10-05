#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "messages.h"
#include "texture_manager.h"

typedef struct {
    uint8_t id;
    uint16_t width, height;
    size_t pixelsWritten;
    Mgpu_Color *pixels;
} Texture;

struct Mgpu_TextureManager {
    Mgpu_Allocator *allocator;
    size_t textureCount;
    Texture *textures;
};

void free_texture(Texture *texture, Mgpu_Allocator *allocator) {
    assert(texture != NULL);
    assert(allocator != NULL);

    allocator->FreeFn(texture->pixels);
    texture->width = 0;
    texture->height = 0;
    texture->pixels = NULL;
}

/*
 * Either gets the index of the texture with the specified id, or gets the
 * index where the texture should be inserted.
 */
size_t get_potential_index_of_id(Mgpu_TextureManager *manager, uint8_t textureId, Texture **exactMatch) {
    assert(manager != NULL);
    assert(exactMatch != NULL);

    // Should probably convert this to binary search at some point, though currently
    // only 255 ids can be used for textures.
    *exactMatch = NULL;
    for (size_t i = 0; i < manager->textureCount; i++) {
        Texture *texture = &manager->textures[i];
        if (texture->id == textureId) {
            *exactMatch = texture;
            return i;
        }

        if (texture->id > textureId) {
            return i;
        }
    }

    // No match found, needs to get added to the end of the collection
    return manager->textureCount;
}

void add_new_texture(Mgpu_TextureManager *textureManager,
                     size_t addAtIndex,
                     uint8_t id,
                     uint16_t width,
                     uint16_t height) {
    if (width == 0 || height == 0) {
        // Removing an undefined texture
        return;
    }

    // Create a new collection with space for the new texture.
    size_t afterCount = textureManager->textureCount + 1;
    Texture *textures = textureManager->allocator->AllocateFn(sizeof(Texture) * afterCount);
    if (textures == NULL) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Defining texture id %u failed: failed to allocate additional texture space", id);
        mgpu_message_set(msg);

        return;
    }

    Mgpu_Color *pixels = textureManager->allocator->AllocateFn(sizeof(Mgpu_Color) * height * width);
    if (pixels == NULL) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Defining texture id %u failed: failed to allocate pixel space", id);
        mgpu_message_set(msg);

        return;
    }

    size_t bytesBeforeIndex = sizeof(Texture) * addAtIndex;
    size_t bytesAfterIndex = sizeof(Texture) * (afterCount - addAtIndex - 1);
    memcpy_s(textures,
             bytesBeforeIndex,
             textureManager->textures,
             bytesBeforeIndex);

    if (bytesAfterIndex > 0) {
        memcpy_s(&textures[addAtIndex + 1],
                 bytesAfterIndex,
                 &textureManager->textures[addAtIndex],
                 bytesAfterIndex);

    }

    textureManager->allocator->FreeFn(textureManager->textures);
    textureManager->textures = textures;

    Texture *texture = &textureManager->textures[addAtIndex];
    texture->id = id;
    texture->width = width;
    texture->height = height;
    texture->pixelsWritten = 0;
    texture->pixels = pixels;
}

void remove_texture(Mgpu_TextureManager *textureManager, size_t removeAtIndex) {
#error TODO
}

void update_texture(Texture *texture, uint16_t width, uint16_t height, Mgpu_Allocator *allocator) {
#error TODO
}

Mgpu_TextureManager *mgpu_texture_manager_new(Mgpu_Allocator *allocator) {
    assert(allocator != NULL);

    Mgpu_TextureManager *manager = allocator->AllocateFn(sizeof(Mgpu_TextureManager));
    if (manager == NULL) {
        mgpu_message_set("Failed to allocate texture manager");
        return NULL;
    }

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

void mgpu_texture_define(Mgpu_TextureManager *textureManager, uint8_t id, uint16_t width, uint16_t height) {
    assert(textureManager != NULL);

    Texture *texture = NULL;
    size_t index = get_potential_index_of_id(textureManager, id, &texture);
    if (texture == NULL) {
        // Texture doesn't exist
        add_new_texture(textureManager, index, id, width, height);
        return;
    }

    // Texture was found
    if (width == 0 || height == 0) {
        // Texture is being removed
        remove_texture(textureManager, index);
        return;
    }

    update_texture(texture, width, height, textureManager->allocator);
}

void mgpu_texture_append(Mgpu_TextureManager *textureManager, uint8_t id, uint16_t pixelCount, Mgpu_Color *pixels) {
    assert(textureManager != NULL);

#error TODO

}




