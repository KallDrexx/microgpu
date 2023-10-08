#pragma once

#include <stdint.h>
#include "color.h"
#include "framebuffer.h"
#include "display.h"

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
     * Sets the number of textures that will be defined. Once set, texture ids
     * 0 to N can be defined, where `N` is one less than the texture count specified.
     *
     * Any textures that have been previously defined are cleared on this operation's
     * execution.
     */
    Mgpu_Operation_SetTextureCount = 8,

    /*
     * Defines a texture the GPU should track. Gives the identifier and dimensions
     * of the texture. If called with an identifier that was previously defined,
     * the previous texture is cleared and a new one is started. If the new
     * width and height are zeros, then the texture is considered undefined.
     */
    Mgpu_Operation_DefineTexture = 9,

    /*
     * Appends bytes representing pixel color data to a defined texture's data. Bytes are
     * expected to be raw color data based on the firmware's active color mode.
     */
    Mgpu_Operation_AppendTexturePixels = 10,

    /*
     * Renders a texture directly to the frame buffer.
     */
    Mgpu_Operation_RenderTexture = 11,

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
} Mgpu_DrawRectangleOperation;

typedef struct {
    uint16_t x0, y0, x1, y1, x2, y2;
    Mgpu_Color color;
} Mgpu_DrawTriangleOperation;

typedef struct {
    uint16_t byteLength;
    const uint8_t *bytes;
} Mgpu_BatchOperation;

typedef struct {
    uint8_t textureCount;
} Mgpu_SetTextureCountOperation;

typedef struct {
    uint8_t textureId;
    uint16_t width, height;
    Mgpu_Color transparentColor;
} Mgpu_DefineTextureOperation;

typedef struct {
    uint8_t textureId;
    uint16_t pixelCount;
    Mgpu_Color *pixels;
} Mgpu_AppendTexturePixelOperation;

typedef struct {
    uint8_t textureId;

    /*
     * The x position to display the top left corner of the texture
     */
    int16_t xPosition;

    /*
     * The y position to display the top left corner of the texture
     */
    int16_t yPosition;
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
        Mgpu_SetTextureCountOperation setTextureCount;
        Mgpu_DefineTextureOperation defineTexture;
        Mgpu_AppendTexturePixelOperation appendTexturePixels;
        Mgpu_DrawTextureOperation drawTexture;
    };
} Mgpu_Operation;
