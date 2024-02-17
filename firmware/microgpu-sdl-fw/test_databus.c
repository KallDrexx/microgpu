#include <stdlib.h>
#include <assert.h>
#include <SDL.h>
#include "microgpu-common/operations/operation_deserializer.h"
#include "microgpu-common/fonts/fonts.h"
#include "test_databus.h"

#define RESET_OPERATION_ID 250
#define TEST_TEXTURE_PIXEL_COUNT 50

bool hasResponse;
Mgpu_Response lastSeenResponse;
uint16_t operationCount;
char testString[] = "Hello world!";

uint8_t testTexturePixels[TEST_TEXTURE_PIXEL_COUNT * TEST_TEXTURE_PIXEL_COUNT * 2];

Mgpu_Databus *mgpu_databus_new(Mgpu_DatabusOptions *options, const Mgpu_Allocator *allocator) {
    assert(options != NULL);
    mgpu_alloc_assert(allocator);

    Mgpu_Databus *databus = allocator->FastMemAllocateFn(sizeof(Mgpu_Databus));
    databus->allocator = allocator;

    if (databus == NULL) {
        return NULL;
    }

    hasResponse = false;
    operationCount = 0;

    // Generate the test texture values in columns of red->white->green->white->blue,
    // with a yellow top and purple bottom
#define COL_WIDTH (TEST_TEXTURE_PIXEL_COUNT / 5)
    uint8_t *pixelByte = testTexturePixels;
    for (int row = 0; row < TEST_TEXTURE_PIXEL_COUNT; row++) {
        for (int col = 0; col < TEST_TEXTURE_PIXEL_COUNT; col++) {
            Mgpu_Color color;
            if (col == 0) {
                color = mgpu_color_from_rgb888(0, 255, 0);
            } else if (col == TEST_TEXTURE_PIXEL_COUNT - 1) {
                color = mgpu_color_from_rgb565(0, 63, 30);
            } else if (row == 0) {
                color = mgpu_color_from_rgb565(31, 62, 0);
            } else if (row == TEST_TEXTURE_PIXEL_COUNT - 1) {
                color = mgpu_color_from_rgb565(30, 0, 31);
            } else if (col < COL_WIDTH) {
                color = mgpu_color_from_rgb888(255, 0, 0);
            } else if (col < COL_WIDTH * 2) {
                color = mgpu_color_from_rgb888(255, 255, 255);
            } else if (col < COL_WIDTH * 3) {
                color = mgpu_color_from_rgb888(0, 255, 0);
            } else if (col < COL_WIDTH * 4) {
                color = mgpu_color_from_rgb888(255, 255, 255);
            } else {
                color = mgpu_color_from_rgb888(0, 0, 255);
            }

            *pixelByte = color >> 8;
            *(pixelByte + 1) = color & 0xFF;
            pixelByte += 2;
        }
    }

    return databus;
}

void mgpu_databus_free(Mgpu_Databus *databus) {
    if (databus != NULL) {
        databus->allocator->FastMemFreeFn(databus);
    }
}

bool mgpu_databus_get_next_operation(Mgpu_Databus *databus, Mgpu_Operation *operation) {
    assert(operation != NULL);

    switch (operationCount) {
        case 0:
            operation->type = Mgpu_Operation_GetStatus;
            operationCount++;
            return true;

        case 1:
            operation->type = Mgpu_Operation_Initialize;
            operation->initialize.frameBufferScale = 1;
            operationCount++;
            return true;

        case 2:
            operation->type = Mgpu_Operation_GetStatus;
            operationCount++;
            return true;

        case 3:
            operation->type = Mgpu_Operation_DrawRectangle;
            operation->drawRectangle.startX = 10;
            operation->drawRectangle.startY = 20;
            operation->drawRectangle.width = 50;
            operation->drawRectangle.height = 20;
            operation->drawRectangle.color = mgpu_color_from_rgb888(255, 0, 0);
            operation->drawRectangle.textureId = 0;
            operationCount++;
            return true;

        case 4:
            operation->type = Mgpu_Operation_DrawTriangle;
            operation->drawTriangle.x0 = 60;
            operation->drawTriangle.y0 = 10;
            operation->drawTriangle.x1 = 30;
            operation->drawTriangle.y1 = 100;
            operation->drawTriangle.x2 = 90;
            operation->drawTriangle.y2 = 100;
            operation->drawTriangle.color = mgpu_color_from_rgb888(0, 255, 0);
            operation->drawTriangle.textureId = 0;
            operationCount++;
            return true;

        case 5:
            operation->type = 999;
            operationCount++;
            return true;

        case 6:
            operation->type = Mgpu_Operation_GetLastMessage;
            operationCount++;
            return true;

        case 7:
            operation->type = Mgpu_Operation_PresentFramebuffer;
            operationCount++;
            return true;

        case 8:
            operation->type = Mgpu_Operation_DrawRectangle;
            operation->drawRectangle.startX = 100;
            operation->drawRectangle.startY = 200;
            operation->drawRectangle.width = 50;
            operation->drawRectangle.height = 20;
            operation->drawRectangle.color = mgpu_color_from_rgb888(0, 0, 255);
            operation->drawRectangle.textureId = 0;
            operationCount++;
            return true;

        case 9:
            operation->type = Mgpu_Operation_DrawTriangle;
            operation->drawTriangle.x0 = 60;
            operation->drawTriangle.y0 = 10;
            operation->drawTriangle.x1 = 30;
            operation->drawTriangle.y1 = 100;
            operation->drawTriangle.x2 = 90;
            operation->drawTriangle.y2 = 100;
            operation->drawTriangle.color = mgpu_color_from_rgb888(120, 120, 120);
            operation->drawTriangle.textureId = 0;
            operationCount++;
            return true;

        case 10: {
            // Two rectangles side by side using a batch
            uint8_t bytes[] = {
                    0x00, 0x0c, 0x02, 0x00, 0x00, 0xc8, 0x00, 0xc8, 0x00, 0x32, 0x00, 0x14, 0xf8, 0x00,
                    0x00, 0x0c, 0x02, 0x00, 0x01, 0x90, 0x00, 0xc8, 0x00, 0x32, 0x00, 0x14, 0x07, 0xe0,
            };

            uint8_t *buffer = malloc(26);
            memmove(buffer, bytes, 26);

            operation->type = Mgpu_Operation_Batch;
            operation->batchOperation.byteLength = 28;
            operation->batchOperation.bytes = buffer;

            operationCount++;
            return true;
        }

        case 11:
            operation->type = Mgpu_Operation_DefineTexture;
            operation->defineTexture.textureId = 5;
            operation->defineTexture.width = TEST_TEXTURE_PIXEL_COUNT;
            operation->defineTexture.height = TEST_TEXTURE_PIXEL_COUNT;
            operation->defineTexture.transparentColor = mgpu_color_from_rgb888(255, 255, 255);
            operationCount++;
            return true;

        case 12:
            operation->type = Mgpu_Operation_AppendTexturePixels;
            operation->appendTexturePixels.textureId = 5;
            operation->appendTexturePixels.pixelCount = sizeof(testTexturePixels) / sizeof(Mgpu_Color);
            operation->appendTexturePixels.pixelBytes = testTexturePixels;
            operationCount++;
            return true;

        case 13:
            operation->type = Mgpu_Operation_DrawTexture;
            operation->drawTexture.sourceTextureId = 5;
            operation->drawTexture.targetTextureId = 0;
            operation->drawTexture.sourceStartX = 0;
            operation->drawTexture.sourceStartY = 0;
            operation->drawTexture.sourceWidth = TEST_TEXTURE_PIXEL_COUNT;
            operation->drawTexture.sourceHeight = TEST_TEXTURE_PIXEL_COUNT;
            operation->drawTexture.targetStartX = 50;
            operation->drawTexture.targetStartY = 50;

            operationCount++;
            return true;

        case 14:
            operation->type = Mgpu_Operation_DrawChars;
            operation->drawChars.fontId = Mgpu_Font_Font8x12;
            operation->drawChars.textureId = 0;
            operation->drawChars.color = mgpu_color_from_rgb888(255, 255, 255);
            operation->drawChars.startX = 100;
            operation->drawChars.startY = 300;
            operation->drawChars.numCharacters = strlen(testString);
            operation->drawChars.characters = (const uint8_t*) testString;

            operationCount++;
            return true;

        case 15:
            operation->type = Mgpu_Operation_DrawChars;
            operation->drawChars.fontId = Mgpu_Font_Font8x12;
            operation->drawChars.textureId = 0;
            operation->drawChars.color = mgpu_color_from_rgb888(255, 0, 0);
            operation->drawChars.startX = 1000;
            operation->drawChars.startY = 300;
            operation->drawChars.numCharacters = strlen(testString);
            operation->drawChars.characters = (const uint8_t*) testString;

            operationCount++;
            return true;

        case 16:
            operation->type = Mgpu_Operation_DrawChars;
            operation->drawChars.fontId = Mgpu_Font_Font8x12;
            operation->drawChars.textureId = 0;
            operation->drawChars.color = mgpu_color_from_rgb888(0, 0, 255);
            operation->drawChars.startX = 100;
            operation->drawChars.startY = 760;
            operation->drawChars.numCharacters = strlen(testString);
            operation->drawChars.characters = (const uint8_t*) testString;

            operationCount++;
            return true;

        case 17:
            operation->type = Mgpu_Operation_DrawChars;
            operation->drawChars.fontId = Mgpu_Font_Font12x16;
            operation->drawChars.textureId = 0;
            operation->drawChars.color = mgpu_color_from_rgb888(0, 0, 255);
            operation->drawChars.startX = 100;
            operation->drawChars.startY = 320;
            operation->drawChars.numCharacters = strlen(testString);
            operation->drawChars.characters = (const uint8_t*) testString;

            operationCount++;
            return true;

        case 18:
            operation->type = Mgpu_Operation_PresentFramebuffer;
            operationCount++;
            return true;

        case RESET_OPERATION_ID:
            operation->type = Mgpu_Operation_Reset;
            operationCount++;
            return true;

        default:
            SDL_Delay(1000);
            return false;
    }
}

void mgpu_databus_send_response(Mgpu_Databus *databus, Mgpu_Response *response) {
    assert(databus != NULL);
    assert(response != NULL);

    hasResponse = true;
    lastSeenResponse.type = response->type;
    switch (response->type) {
        case Mgpu_Response_Status:
            lastSeenResponse.status = response->status;
            break;

        case Mgpu_Response_LastMessage:
            lastSeenResponse.lastMessage = response->lastMessage;
            break;
    }
}

bool mgpu_test_databus_get_last_response(Mgpu_Databus *databus, Mgpu_Response *response) {
    if (hasResponse) {
        *response = lastSeenResponse;
        hasResponse = false;
        return true;
    }

    return false;
}

void mgpu_test_databus_trigger_reset(Mgpu_Databus *databus) {
    operationCount = RESET_OPERATION_ID;
}

uint16_t mgpu_databus_get_max_size(Mgpu_Databus *databus) {
    assert(databus != NULL);
    return 984;
}
