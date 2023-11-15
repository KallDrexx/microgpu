#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "benchmark_databus.h"
#include "common.h"

uint16_t operationCount;
bool hadPreviousOperation;
Mgpu_OperationType lastOperationType;
int64_t prevOperationStartedAt;
bool hasResponse;
bool hasFinished = false;
Mgpu_Response lastSeenResponse;

bool get_operation(Mgpu_Operation *operation) {
    switch (operationCount) {
        case 0:
            operation->type = Mgpu_Operation_GetStatus;
            return true;

        case 1:
            operation->type = Mgpu_Operation_Initialize;
            operation->initialize.frameBufferScale = 1;
            return true;

        case 2:
            operation->type = Mgpu_Operation_GetStatus;
            return true;

        case 3:
            operation->type = Mgpu_Operation_DrawRectangle;
            operation->drawRectangle.textureId = 0;
            operation->drawRectangle.startX = 10;
            operation->drawRectangle.startY = 20;
            operation->drawRectangle.width = 50;
            operation->drawRectangle.height = 20;
            operation->drawRectangle.color = mgpu_color_from_rgb888(255, 0, 0);
            return true;

        case 4:
            operation->type = Mgpu_Operation_DrawTriangle;
            operation->drawTriangle.textureId = 0;
            operation->drawTriangle.x0 = 60;
            operation->drawTriangle.y0 = 10;
            operation->drawTriangle.x1 = 30;
            operation->drawTriangle.y1 = 100;
            operation->drawTriangle.x2 = 90;
            operation->drawTriangle.y2 = 100;
            operation->drawTriangle.color = mgpu_color_from_rgb888(0, 255, 0);
            return true;

        case 5:
            operation->type = 999;
            return true;

        case 6:
            operation->type = 999;
            return true;

        case 7:
            operation->type = Mgpu_Operation_GetLastMessage;
            return true;

        case 8:
            operation->type = Mgpu_Operation_PresentFramebuffer;
            return true;

        case 9:
            operation->type = Mgpu_Operation_DrawRectangle;
            operation->drawRectangle.textureId = 0;
            operation->drawRectangle.startX = 100;
            operation->drawRectangle.startY = 200;
            operation->drawRectangle.width = 50;
            operation->drawRectangle.height = 20;
            operation->drawRectangle.color = mgpu_color_from_rgb888(0, 0, 255);
            return true;

        case 10:
            operation->type = Mgpu_Operation_DrawTriangle;
            operation->drawTriangle.textureId = 0;
            operation->drawTriangle.x0 = 60;
            operation->drawTriangle.y0 = 10;
            operation->drawTriangle.x1 = 30;
            operation->drawTriangle.y1 = 100;
            operation->drawTriangle.x2 = 90;
            operation->drawTriangle.y2 = 100;
            operation->drawTriangle.color = mgpu_color_from_rgb888(0, 255, 0);
            return true;

        case 11:
            operation->type = Mgpu_Operation_PresentFramebuffer;
            return true;

        default:
            return false;
    }
}

Mgpu_Databus *mgpu_databus_new(Mgpu_DatabusOptions *options, const Mgpu_Allocator *allocator) {
    assert(options != NULL);
    assert(allocator != NULL);

    Mgpu_Databus *databus = allocator->AllocateFn(sizeof(Mgpu_Databus));
    databus->allocator = allocator;

    if (databus == NULL) {
        return NULL;
    }

    operationCount = 0;

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
        int stackSpace = uxTaskGetStackHighWaterMark(NULL);
        ESP_LOGI(LOG_TAG,
                 "Operation #%u (type: %u) took %lld microseconds (stack size: %u)",
                 operationCount - 1,
                 lastOperationType,
                 now - prevOperationStartedAt,
                 stackSpace);
    }

    bool result = get_operation(operation);
    if (result) {
        operationCount++;
        hadPreviousOperation = true;
        prevOperationStartedAt = esp_timer_get_time();
        lastOperationType = operation->type;
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
