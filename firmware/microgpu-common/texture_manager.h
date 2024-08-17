#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "alloc.h"
#include "microgpu-common/colors/color.h"

#define NUM_TEXTURES 255

typedef enum {
    /*
     * If set, then the texture should be allocated via the slow ram allocator. Otherwise, the texture should be
     * allocated in fast ram. If the fast ram allocation fails, it will attempt the slow ram.
     */
    MGPU_TEXTURE_USE_SLOW_RAM = 1 << 0,

    /*
     * If set, the texture is encoded via the RLE texture scheme instead of raw bytes.
     */
    MGPU_TEXTURE_RLE_ENCODED = 1 << 1,
} Mgpu_TextureDefinitionFlags;

typedef struct {
    uint8_t id;
    uint16_t width, height;
    uint32_t byteCount;
    Mgpu_TextureDefinitionFlags flags;
} Mgpu_TextureDefinition;

typedef struct {
    uint16_t width, height;
    Mgpu_Color transparencyColor;

    /*
     * Declares how the texture is scaled when drawn. Mostly used for the frame buffer to be
     * scaled when drawn to the display
     */
    uint8_t scale;
    size_t pixelsWritten;
    bool allocatedInSlowRam;
    Mgpu_Color pixels[];
} Mgpu_Texture;

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
 * Defines the properties for a specific texture. If a texture had already been defined
 * with the specified id, then the existing texture is cleared and a new one is allocated
 * in its place. If the width or height are zero, then the texture is cleared and no new
 * one is allocated.
 *
 * Returns false if a texture could not be defined for any reason.
 */
bool mgpu_texture_define(Mgpu_TextureManager *textureManager, Mgpu_TextureDefinition *info, uint8_t scale);

/*
 * Retrieves the texture with the specified id.
 */
Mgpu_Texture *mgpu_texture_get(Mgpu_TextureManager *textureManager, uint8_t id);

/*
 * Swaps two textures so their ids are reversed.
 */
void mgpu_texture_swap(Mgpu_TextureManager *textureManager, uint8_t firstId, uint8_t secondId);
