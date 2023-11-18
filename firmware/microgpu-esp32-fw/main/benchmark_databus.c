#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "benchmark_databus.h"
#include "common.h"

#define NAME_SIZE 50
#define OP_COUNT 30
#define TEST_TEXTURE_PIXEL_COUNT 150

typedef struct {
    Mgpu_Operation operation;
    char name[NAME_SIZE];
} OperationInfo;

uint16_t operationCount;
bool hadPreviousOperation;
int64_t prevOperationStartedAt;
bool hasResponse;
bool hasFinished = false;
Mgpu_Response lastSeenResponse;
OperationInfo operations[OP_COUNT];
uint8_t *testTexturePixels;
size_t lastFreeHeapSize, lastFreeStackSize;

void setup_operations(void) {
    int index = 0;
    snprintf(operations[index].name, NAME_SIZE, "Pre-init status");
    operations[index].operation.type = Mgpu_Operation_GetStatus;

    index++;
    snprintf(operations[index].name, NAME_SIZE, "Initialize");
    operations[index].operation.type = Mgpu_Operation_Initialize;
    operations[index].operation.initialize.frameBufferScale = 1;

    index++;
    snprintf(operations[index].name, NAME_SIZE, "Post-init status");
    operations[index].operation.type = Mgpu_Operation_GetStatus;

    index++;
    snprintf(operations[index].name, NAME_SIZE, "Define texture");
    operations[index].operation.type = Mgpu_Operation_DefineTexture;
    operations[index].operation.defineTexture.textureId = 5;
    operations[index].operation.defineTexture.width = TEST_TEXTURE_PIXEL_COUNT;
    operations[index].operation.defineTexture.height = TEST_TEXTURE_PIXEL_COUNT;
    operations[index].operation.defineTexture.transparentColor = mgpu_color_from_rgb888(255, 255, 255);

    index++;
    snprintf(operations[index].name, NAME_SIZE, "Append texture pixels 1");
    operations[index].operation.type = Mgpu_Operation_AppendTexturePixels;
    operations[index].operation.appendTexturePixels.textureId = 5;
    operations[index].operation.appendTexturePixels.pixelCount =
            (TEST_TEXTURE_PIXEL_COUNT * TEST_TEXTURE_PIXEL_COUNT) / 2;
    operations[index].operation.appendTexturePixels.pixelBytes = testTexturePixels;

    index++;
    snprintf(operations[index].name, NAME_SIZE, "Append texture pixels 2");
    operations[index].operation.type = Mgpu_Operation_AppendTexturePixels;
    operations[index].operation.appendTexturePixels.textureId = 5;
    operations[index].operation.appendTexturePixels.pixelCount =
            (TEST_TEXTURE_PIXEL_COUNT * TEST_TEXTURE_PIXEL_COUNT) / 2;
    operations[index].operation.appendTexturePixels.pixelBytes = testTexturePixels +
                                                                 ((TEST_TEXTURE_PIXEL_COUNT *
                                                                   TEST_TEXTURE_PIXEL_COUNT) / 2 * sizeof(Mgpu_Color));

    index++;
    snprintf(operations[index].name, NAME_SIZE, "Draw 50x20 rectangle");
    operations[index].operation.type = Mgpu_Operation_DrawRectangle;
    operations[index].operation.drawRectangle.textureId = 0;
    operations[index].operation.drawRectangle.startX = 10;
    operations[index].operation.drawRectangle.startY = 20;
    operations[index].operation.drawRectangle.width = 50;
    operations[index].operation.drawRectangle.height = 20;
    operations[index].operation.drawRectangle.color = mgpu_color_from_rgb888(255, 0, 0);

    index++;
    snprintf(operations[index].name, NAME_SIZE, "Draw triangle");
    operations[index].operation.type = Mgpu_Operation_DrawTriangle;
    operations[index].operation.drawTriangle.textureId = 0;
    operations[index].operation.drawTriangle.x0 = 60;
    operations[index].operation.drawTriangle.y0 = 10;
    operations[index].operation.drawTriangle.x1 = 30;
    operations[index].operation.drawTriangle.y1 = 100;
    operations[index].operation.drawTriangle.x2 = 90;
    operations[index].operation.drawTriangle.y2 = 100;
    operations[index].operation.drawTriangle.color = mgpu_color_from_rgb888(0, 255, 0);

    index++;
    snprintf(operations[index].name, NAME_SIZE, "Invalid operation type");
    operations[index].operation.type = 999;

    index++;
    snprintf(operations[index].name, NAME_SIZE, "Get last message");
    operations[index].operation.type = Mgpu_Operation_GetLastMessage;

    index++;
    snprintf(operations[index].name, NAME_SIZE, "Present framebuffer");
    operations[index].operation.type = Mgpu_Operation_PresentFramebuffer;

    index++;
    snprintf(operations[index].name, NAME_SIZE, "Full Size Rectangle");
    operations[index].operation.type = Mgpu_Operation_DrawRectangle;
    operations[index].operation.drawRectangle.textureId = 0;
    operations[index].operation.drawRectangle.startX = 0;
    operations[index].operation.drawRectangle.startY = 0;
    operations[index].operation.drawRectangle.width = 320;
    operations[index].operation.drawRectangle.height = 240;
    operations[index].operation.drawRectangle.color = mgpu_color_from_rgb888(0, 0, 0);

    index++;
    snprintf(operations[index].name, NAME_SIZE, "Draw 50x20 rectangle");
    operations[index].operation.type = Mgpu_Operation_DrawRectangle;
    operations[index].operation.drawRectangle.textureId = 0;
    operations[index].operation.drawRectangle.startX = 100;
    operations[index].operation.drawRectangle.startY = 200;
    operations[index].operation.drawRectangle.width = 50;
    operations[index].operation.drawRectangle.height = 20;
    operations[index].operation.drawRectangle.color = mgpu_color_from_rgb888(0, 0, 255);

    index++;
    snprintf(operations[index].name, NAME_SIZE, "Draw triangle");
    operations[index].operation.type = Mgpu_Operation_DrawTriangle;
    operations[index].operation.drawTriangle.textureId = 0;
    operations[index].operation.drawTriangle.x0 = 60;
    operations[index].operation.drawTriangle.y0 = 10;
    operations[index].operation.drawTriangle.x1 = 30;
    operations[index].operation.drawTriangle.y1 = 100;
    operations[index].operation.drawTriangle.x2 = 90;
    operations[index].operation.drawTriangle.y2 = 100;
    operations[index].operation.drawTriangle.color = mgpu_color_from_rgb888(0, 255, 0);

    index++;
    snprintf(operations[index].name, NAME_SIZE, "Draw full size texture w/transparency");
    operations[index].operation.type = Mgpu_Operation_DrawTexture;
    operations[index].operation.drawTexture.sourceTextureId = 5;
    operations[index].operation.drawTexture.targetTextureId = 0;
    operations[index].operation.drawTexture.sourceStartX = 0;
    operations[index].operation.drawTexture.sourceStartY = 0;
    operations[index].operation.drawTexture.sourceWidth = TEST_TEXTURE_PIXEL_COUNT;
    operations[index].operation.drawTexture.sourceHeight = TEST_TEXTURE_PIXEL_COUNT;
    operations[index].operation.drawTexture.targetStartX = 0;
    operations[index].operation.drawTexture.targetStartY = 0;
    operations[index].operation.drawTexture.ignoreTransparency = false;

    index++;
    snprintf(operations[index].name, NAME_SIZE, "Draw full size texture no transparency");
    operations[index].operation.type = Mgpu_Operation_DrawTexture;
    operations[index].operation.drawTexture.sourceTextureId = 5;
    operations[index].operation.drawTexture.targetTextureId = 0;
    operations[index].operation.drawTexture.sourceStartX = 0;
    operations[index].operation.drawTexture.sourceStartY = 0;
    operations[index].operation.drawTexture.sourceWidth = TEST_TEXTURE_PIXEL_COUNT;
    operations[index].operation.drawTexture.sourceHeight = TEST_TEXTURE_PIXEL_COUNT;
    operations[index].operation.drawTexture.targetStartX = 0;
    operations[index].operation.drawTexture.targetStartY = 0;
    operations[index].operation.drawTexture.ignoreTransparency = true;

    index++;
    snprintf(operations[index].name, NAME_SIZE, "Present framebuffer");
    operations[index].operation.type = Mgpu_Operation_PresentFramebuffer;
}

bool get_operation(Mgpu_Operation *operation) {
    if (operationCount >= OP_COUNT || operations[operationCount].operation.type == 0) {
        return false;
    }

    memcpy(operation, &operations[operationCount].operation, sizeof(Mgpu_Operation));

    return true;
}

Mgpu_Databus *mgpu_databus_new(Mgpu_DatabusOptions *options, const Mgpu_Allocator *allocator) {
    assert(options != NULL);
    assert(allocator != NULL);

    size_t freeHeap = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    ESP_LOGI(LOG_TAG, "Free heap before databus creation: %u", freeHeap);

    Mgpu_Databus *databus = allocator->AllocateFn(sizeof(Mgpu_Databus));
    databus->allocator = allocator;

    if (databus == NULL) {
        return NULL;
    }

    testTexturePixels = allocator->AllocateFn(TEST_TEXTURE_PIXEL_COUNT * TEST_TEXTURE_PIXEL_COUNT * sizeof(Mgpu_Color));

    operationCount = 0;
    setup_operations();

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

    size_t freeHeap2 = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    ESP_LOGI(LOG_TAG, "Free heap after databus creation: %u (diff: %d)", freeHeap2, freeHeap - freeHeap2);

    lastFreeHeapSize = freeHeap2;
    lastFreeStackSize = uxTaskGetStackHighWaterMark(NULL);
    return databus;
}

void mgpu_databus_free(Mgpu_Databus *databus) {
    if (databus != NULL) {
        databus->allocator->FreeFn(databus);
    }
}


bool mgpu_databus_get_next_operation(Mgpu_Databus *databus, Mgpu_Operation *operation) {
    assert(operation != NULL);

    if (hadPreviousOperation) {
        int64_t now = esp_timer_get_time();
        size_t freeStack = uxTaskGetStackHighWaterMark(NULL);
        size_t freeHeap = heap_caps_get_free_size(MALLOC_CAP_8BIT);
        int stackDiff = (int) freeStack - (int) lastFreeStackSize;
        int heapDiff = (int) freeHeap - (int) lastFreeHeapSize;

        ESP_LOGI(LOG_TAG,
                 "Operation #%u (%s) took %lld microseconds, free stack/heap: %u (%d) /  %u (%d)",
                 operationCount - 1,
                 operations[operationCount - 1].name,
                 now - prevOperationStartedAt,
                 freeStack,
                 stackDiff,
                 freeHeap,
                 heapDiff);

        lastFreeStackSize = freeStack;
        lastFreeHeapSize = freeHeap;
    }

    bool result = get_operation(operation);
    if (result) {
        operationCount++;
        hadPreviousOperation = true;
        prevOperationStartedAt = esp_timer_get_time();
    } else {
        hadPreviousOperation = false;
        if (!hasFinished) {
            ESP_LOGI(LOG_TAG, "Finished all operations");
            hasFinished = true;
        }
        vTaskDelay(1000);
    }

    return result;
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

uint16_t mgpu_databus_get_max_size(Mgpu_Databus *databus) {
    assert(databus != NULL);

    return 0;
}
