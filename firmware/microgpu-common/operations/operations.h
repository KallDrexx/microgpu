#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "microgpu-common/colors/color.h"
#include "microgpu-common/display.h"

/*
 * What type of operations are supported
 */
typedef enum {
    /*
     * Operation to initialize the microgpu system. Must be provided before
     * most other operations can operate and has no effect after a previous
     * initialization call.
     */
    Mgpu_Operation_Initialize = 1,

    /*
     * Operation to draw a filled in rectangle to the framebuffer.
     */
    Mgpu_Operation_DrawRectangle = 2,

    /*
     * Operation to draw a filled in triangle between three defined points.
     */
    Mgpu_Operation_DrawTriangle = 3,

    /*
     * Requests getting the status of the microgpu system. Can be requested
     * prior to initialization, which is useful for getting the display
     * resolution to compute framebuffer scale.
     */
    Mgpu_Operation_GetStatus = 4,

    /*
     * Requests the latest message raised by the microgpu system, which usually
     * corresponds to the previously executed operation.
     */
    Mgpu_Operation_GetLastMessage = 5,

    /*
     * Sends the frame buffer to the display for presentation.
     */
    Mgpu_Operation_PresentFramebuffer = 6,

    /*
     * A single operation that contains one or more operations inside of it. Used to
     * reduce overhead in some transports.
     */
    Mgpu_Operation_Batch = 7,

    /*
     * Defines a texture the GPU should track. Gives the identifier and dimensions
     * of the texture. If called with an identifier that was previously defined,
     * the previous texture is cleared and a new one is started. If the new
     * width and height are zeros, then the texture is considered undefined.
     *
     * Only texture ids 1-230 are allowed to be defined externally, the rest of them
     * are reserved for internal usage. Texture id 0 is always the currently active
     * frame buffer.
     */
    Mgpu_Operation_DefineTexture = 9,

    /*
     * Appends bytes representing pixel color data to a defined texture's data. Bytes are
     * expected to be raw color data based on the firmware's active color mode.
     */
    Mgpu_Operation_AppendTexturePixels = 10,

    /*
     * Draws a portion of one texture onto another
     */
    Mgpu_Operation_DrawTexture = 11,

    /*
     * Requests the microgpu to initialize itself and fully reset itself.
     */
    Mgpu_Operation_Reset = 189, // Higher value that's hard to see accidentally
} Mgpu_OperationType;

typedef struct {
    /*
     * How much to scale the frame buffer down from the display resolution.
     * E.g. if the display is 320x240 and a scale of 2 is provided, the frame
     * buffer will have a resolution of 160x120, and will be scaled up to 320x240
     * at render time.
     *
     * A scale factor of 0 is invalid
     */
    uint8_t frameBufferScale;
} Mgpu_InitializeOperation;

typedef struct {
    uint16_t startX, startY, width, height;
    Mgpu_Color color;
    uint8_t textureId;
} Mgpu_DrawRectangleOperation;

typedef struct {
    uint16_t x0, y0, x1, y1, x2, y2;
    Mgpu_Color color;
    uint8_t textureId;
} Mgpu_DrawTriangleOperation;

typedef struct {
    uint16_t byteLength;
    const uint8_t *bytes;
} Mgpu_BatchOperation;

typedef struct {
    uint8_t textureId;
    uint16_t width, height;
    Mgpu_Color transparentColor;
} Mgpu_DefineTextureOperation;

typedef struct {
    uint8_t textureId;
    uint16_t pixelCount;
    const uint8_t *pixelBytes;
} Mgpu_AppendTexturePixelOperation;

typedef struct {
    /*
     * The texture to pull pixels from
     */
    uint8_t sourceTextureId;

    /*
     * The texture to draw pixels to. Specifying texture id 0 means to draw to
     * the active frame buffer.
     */
    uint8_t targetTextureId;

    /*
     * If true, any of the pixels from the source texture that have the same color
     * as the source texture's transparency color will not be drawn to the target
     * texture.
     */
    bool ignoreTransparency;

    /*
     * The X position to start pulling pixels from on the source texture
     */
    uint16_t sourceStartX;

    /*
     * The Y position to start pulling pixels from on the source texture
     */
    uint16_t sourceStartY;

    /*
     * How many pixels per row to pull from the source texture
     */
    uint16_t sourceWidth;

    /*
     * How many pixels per column to pull from the source texture
     */
    uint16_t sourceHeight;

    /*
     * The X position on the target texture to start drawing
     */
    int16_t targetStartX;

    /*
     * The Y position on the target texture to start drawing
     */
    int16_t targetStartY;
} Mgpu_DrawTextureOperation;

/*
 * Single type that can represent any type of operation that
 * the microgpu framework can support.
 */
typedef struct {
    Mgpu_OperationType type;
    union {
        Mgpu_InitializeOperation initialize;
        Mgpu_DrawRectangleOperation drawRectangle;
        Mgpu_DrawTriangleOperation drawTriangle;
        Mgpu_BatchOperation batchOperation;
        Mgpu_DefineTextureOperation defineTexture;
        Mgpu_AppendTexturePixelOperation appendTexturePixels;
        Mgpu_DrawTextureOperation drawTexture;
    };
} Mgpu_Operation;
