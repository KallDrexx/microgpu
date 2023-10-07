#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "messages.h"
#include "texture_manager.h"

typedef struct {
    uint8_t id;
    uint16_t width, height;
    Mgpu_Color transparentColor;
    size_t pixelsWritten;
    Mgpu_Color *pixels;
} Texture;

struct Mgpu_TextureManager {
    const Mgpu_Allocator *allocator;
    size_t textureCount;
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

void add_new_texture(Mgpu_TextureManager *textureManager, size_t addAtIndex, Mgpu_TextureInfo *info) {
    assert(info != NULL);

    if (info->width == 0 || info->height == 0) {
        // Removing an undefined texture
        return;
    }

    // Create a new collection with space for the new texture.
    size_t afterCount = textureManager->textureCount + 1;
    Texture *textures = textureManager->allocator->AllocateFn(sizeof(Texture) * afterCount);
    if (textures == NULL) {
        char msg[256];
        snprintf(msg,
                 sizeof(msg),
                 "Defining texture id %u failed: failed to allocate additional texture space",
                 info->id);
        mgpu_message_set(msg);

        return;
    }

    Mgpu_Color *pixels = textureManager->allocator->AllocateFn(sizeof(Mgpu_Color) * info->height * info->width);
    if (pixels == NULL) {
        char msg[256];
        snprintf(msg,
                 sizeof(msg),
                 "Defining texture id %u failed: failed to allocate pixel space",
                 info->id);

        mgpu_message_set(msg);

        return;
    }

    size_t bytesBeforeIndex = sizeof(Texture) * addAtIndex;
    size_t bytesAfterIndex = sizeof(Texture) * (afterCount - addAtIndex - 1);
    memcpy(textures, textureManager->textures, bytesBeforeIndex);

    if (bytesAfterIndex > 0) {
        memcpy(&textures[addAtIndex + 1], &textureManager->textures[addAtIndex], bytesAfterIndex);
    }

    textureManager->allocator->FreeFn(textureManager->textures);
    textureManager->textures = textures;

    Texture *texture = &textureManager->textures[addAtIndex];
    texture->id = info->id;
    texture->width = info->width;
    texture->height = info->height;
    texture->pixelsWritten = 0;
    texture->pixels = pixels;
    texture->transparentColor = info->transparentColor;
}

void remove_texture(Mgpu_TextureManager *manager, size_t removeAtIndex) {
    assert(manager != NULL);
    assert(removeAtIndex >= manager->textureCount);

    // If this is the last texture, just free the space
    if (manager->textureCount == 1) {
        manager->textureCount = 0;
        manager->allocator->FreeFn(manager->textures);
        manager->textures = NULL;

        return;
    }

    size_t afterCount = manager->textureCount - 1;
    manager->allocator->FreeFn(manager->textures[removeAtIndex].pixels);
    manager->textures[removeAtIndex].pixels = NULL;

    // Attempt to allocate smaller space for the texture collection. We could maybe
    // lower the number of allocations by keeping capacity, but we have no obvious way
    // to know when we can shrink the collection. So take the extra allocation hit in
    // order to free up the memory for other uses in case this the removal of this
    // texture does not precede new textures being added.
    Texture *newSpace = manager->allocator->AllocateFn(sizeof(Texture) * afterCount);
    Texture *oldSpace = manager->textures;
    if (newSpace == NULL) {
        // We could not allocate enough space. Invalidate the texture and leave it, hoping
        // an update call tries to redefine that same texture id. The texture header is small
        // enough that this *shouldn't* be an issue ever.
        manager->textures[removeAtIndex].width = 0;
        manager->textures[removeAtIndex].height = 0;
        manager->textures[removeAtIndex].pixelsWritten = 0;

        return;
    }

    // Move the non-removed texture headers to the new memory location
    size_t bytesBeforeIndex = sizeof(Texture) * removeAtIndex;
    size_t bytesAfterIndex = sizeof(Texture) * (afterCount - removeAtIndex - 1);
    memcpy(newSpace, manager->textures, bytesBeforeIndex);
    memcpy(newSpace + bytesBeforeIndex, oldSpace + (bytesBeforeIndex + sizeof(Texture)), bytesAfterIndex);

    manager->textureCount = afterCount;
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
        texture->width = 0;
        texture->height = 0;

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

void mgpu_texture_define(Mgpu_TextureManager *textureManager, Mgpu_TextureInfo *info) {
    assert(textureManager != NULL);
    assert(info != NULL);

    Texture *texture = NULL;
    size_t index = get_potential_index_of_id(textureManager, info->id, &texture);
    if (texture == NULL) {
        // Texture doesn't exist
        add_new_texture(textureManager, index, info);
        return;
    }

    // Texture was found
    if (info->width == 0 || info->height == 0) {
        // Texture is being removed
        remove_texture(textureManager, index);
        return;
    }

    update_texture(texture, info, textureManager->allocator);
}

void mgpu_texture_append(Mgpu_TextureManager *textureManager, uint8_t id, uint16_t pixelCount, Mgpu_Color *pixels) {
    assert(textureManager != NULL);
    assert(pixels != NULL);

    Texture *texture = NULL;
    get_potential_index_of_id(textureManager, id, &texture);
    if (texture == NULL) {
        char msg[256];
        snprintf(msg,
                 sizeof(msg),
                 "Attempted to add pixels to texture %d, but that texture is not defined",
                 id);

        return;
    }

    size_t pixelsLeft = (texture->width * texture->height) - texture->pixelsWritten;
    size_t pixelsToWrite = min(pixelsLeft, pixelCount);
    if (pixelsToWrite == 0) {
        return;
    }

    Mgpu_Color *startPos = texture->pixels + texture->pixelsWritten;
    memcpy(startPos, pixels, pixelsToWrite);
}

void mgpu_texture_draw(Mgpu_TextureManager *textureManager,
                       Mgpu_FrameBuffer *frameBuffer,
                       uint16_t textureId,
                       int16_t xPosition,
                       int16_t yPosition) {
    assert(textureManager != NULL);
    assert(frameBuffer != NULL);
    assert(frameBuffer->pixels != NULL);

    Texture *texture = NULL;
    get_potential_index_of_id(textureManager, textureId, &texture);
    if (texture == NULL || texture->pixels == NULL || texture->width == 0 || texture->height == 0) {
        return; // No texture defined
    }

    uint16_t startX = max(xPosition, 0);
    uint16_t startY = max(yPosition, 0);
    uint16_t lastX = min(xPosition + texture->width, frameBuffer->width - 1);
    uint16_t lastY = min(yPosition + texture->height, frameBuffer->height - 1);
    uint16_t width = lastX - startX + 1;
    uint16_t height = lastY - startY + 1;

    Mgpu_Color *sourceRowStart = texture->pixels + (startX - xPosition);
    Mgpu_Color *targetRowStart = frameBuffer->pixels + startX;

    for (int row = 0; row < height; height++) {
        Mgpu_Color *source = sourceRowStart;
        Mgpu_Color *target = targetRowStart;

        for (int col = 0; col < width; col++) {
            *target = *source;
            target++;
            source++;
        }

        sourceRowStart += texture->width;
        targetRowStart += frameBuffer->width;
    }
}
