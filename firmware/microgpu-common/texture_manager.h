#pragma once

#include <stdint.h>
#include "alloc.h"
#include "color.h"
#include "framebuffer.h"

typedef struct {
    uint8_t id;
    uint16_t width, height;
    Mgpu_Color transparentColor;
} Mgpu_TextureInfo;

typedef struct Mgpu_TextureManager Mgpu_TextureManager;

/*
 * Creates a new texture manager instance. The allocator provided will not only be
 * used to allocate the texture manager itself, but also all textures that get
 * defined.
 */
Mgpu_TextureManager *mgpu_texture_manager_new(const Mgpu_Allocator *allocator);

/*
 * Uninitializes and frees memory used by the texture manager. It gets freed
 * by the allocator the texture manager was created from.
 */
void mgpu_texture_manager_free(Mgpu_TextureManager *textureManager);

/*
 * Sets the number of textures to be tracked. When called, all previously defined
 * textures become cleared.
 */
void mgpu_texture_set_count(Mgpu_TextureManager *textureManager, uint8_t count);

/*
 * Defines the properties for a specific texture. If a texture had already been defined
 * with the specified id, then the existing texture is cleared and a new one is allocated
 * in its place. If the width or height are zero, then the texture is cleared and no new
 * one is allocated.
 */
void mgpu_texture_define(Mgpu_TextureManager *textureManager, Mgpu_TextureInfo *info);

/*
 * Appends the pixelBytes to a previously defined texture.
 */
void
mgpu_texture_append(Mgpu_TextureManager *textureManager, uint8_t id, uint16_t pixelCount, const uint8_t *pixelBytes);

/*
 * Draws the specified texture to the frame buffer
 */
void mgpu_texture_draw(Mgpu_TextureManager *textureManager,
                       Mgpu_FrameBuffer *frameBuffer,
                       uint16_t textureId,
                       int16_t xPosition,
                       int16_t yPosition);