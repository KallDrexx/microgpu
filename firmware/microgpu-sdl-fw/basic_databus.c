#include <assert.h>
#include <SDL.h>
#include "basic_databus.h"

#define RESET_OPERATION_ID 250

bool hasResponse;
Mgpu_Response lastSeenResponse;
uint16_t operationCount;

Mgpu_Databus *mgpu_databus_new(Mgpu_DatabusOptions *options, const Mgpu_Allocator *allocator) {
    assert(options != NULL);
    assert(allocator != NULL);

    Mgpu_Databus *databus = allocator->AllocateFn(sizeof(Mgpu_Databus));
    databus->allocator = allocator;

    if (databus == NULL) {
        return NULL;
    }

    hasResponse = false;
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

bool mgpu_basic_databus_get_last_response(Mgpu_Databus *databus, Mgpu_Response *response) {
    if (hasResponse) {
        *response = lastSeenResponse;
        hasResponse = false;
        return true;
    }

    return false;
}

void mgpu_basic_databus_trigger_reset(Mgpu_Databus *databus) {
    operationCount = RESET_OPERATION_ID;
}
