#include <stdbool.h>
#include <microgpu-common/operations/drawing/triangle.h>
#include "esp_log.h"
#include "microgpu-common/alloc.h"
#include "microgpu-common/databus.h"
#include "microgpu-common/display.h"
#include "microgpu-common/framebuffer.h"
#include "microgpu-common/operation_execution.h"
#include "common.h"
#include "displays/i80_display.h"

#ifdef DATABUS_SPI
#include "spi_databus.h"
#elif defined(DATABUS_TEST)
#include "test_databus.h"
#else
#error "No databus set"
#endif

Mgpu_FrameBuffer *frameBuffer;
Mgpu_Display *display;
Mgpu_DisplayOptions displayOptions;
Mgpu_Databus *databus;
Mgpu_DatabusOptions databusOptions;
bool resetRequested;

static const Mgpu_Allocator standardAllocator = {
        .AllocateFn = malloc,
        .FreeFn = free,
};

#ifdef DATABUS_SPI
void init_databus_options() {
    ESP_LOGI(LOG_TAG, "Initializing SPI databus");
    databusOptions.copiPin = 14;
    databusOptions.cipoPin = 15;
    databusOptions.sclkPin = 16;
    databusOptions.csPin = 17;
    databusOptions.handshakePin = 18;
    databusOptions.spiHost = SPI2_HOST;
}
#elif defined(DATABUS_TEST)
void init_databus_options() {
    ESP_LOGI(LOG_TAG, "Initializing test databus");
}

void handleResponse(Mgpu_Response *response) {
    switch (response->type) {
        case Mgpu_Response_Status:
            ESP_LOGI(LOG_TAG, "Status received");
            ESP_LOGI(LOG_TAG, "Display: %ux%u", response->status.displayWidth, response->status.displayHeight);
            ESP_LOGI(LOG_TAG, "Framebuffer: %ux%u", response->status.frameBufferWidth, response->status.frameBufferHeight);
            ESP_LOGI(LOG_TAG, "Color mode: %u", response->status.colorMode);
            ESP_LOGI(LOG_TAG, "Is Initialized: %u", response->status.isInitialized);
            break;

        case Mgpu_Response_LastMessage:
            ESP_LOGI(LOG_TAG, "GetLastMessage response received: %s", response->lastMessage.message);
            break;

        default:
            break;
    }
}
#endif

bool setup(void) {
    ESP_LOGI(LOG_TAG, "Initializing display");

    // TODO: Make most of this settable in config options
    displayOptions.pixelWidth = 320;
    displayOptions.pixelHeight = 240;
    displayOptions.controlPins.reset = 2;
    displayOptions.controlPins.chipSelect = 3;
    displayOptions.controlPins.dataCommand = 4;
    displayOptions.controlPins.writeClock = 5;
    displayOptions.dataPins.data0 = 6;
    displayOptions.dataPins.data1 = 7;
    displayOptions.dataPins.data2 = 8;
    displayOptions.dataPins.data3 = 9;
    displayOptions.dataPins.data4 = 10;
    displayOptions.dataPins.data5 = 11;
    displayOptions.dataPins.data6 = 12;
    displayOptions.dataPins.data7 = 13;

    display = mgpu_display_new(&standardAllocator, &displayOptions);
    if (display == NULL) {
        ESP_LOGE(LOG_TAG, "Display could not be created");
        return false;
    }

    init_databus_options();
    databus = mgpu_databus_new(&databusOptions, &standardAllocator);
    if (databus == NULL) {
        ESP_LOGE(LOG_TAG, "Databus could not be created");
        return false;
    }

    return true;
}

bool wait_for_initialization(void) {
    ESP_LOGI(LOG_TAG, "Waiting for initialization operation");
    Mgpu_Operation operation;

    while (true) {
        bool hasOperation = mgpu_databus_get_next_operation(databus, &operation);
        if (hasOperation) {
            if (operation.type == Mgpu_Operation_Initialize) {
                break;
            }
        }

        // Before initialization, we can only respond to get status and get last message
        if (operation.type == Mgpu_Operation_GetStatus || operation.type == Mgpu_Operation_GetLastMessage) {
            mgpu_execute_operation(&operation, frameBuffer, display, databus, &resetRequested, NULL);
#ifdef DATABUS_TEST
            Mgpu_Response response;
            if (mgpu_test_databus_get_last_response(databus, &response)) {
                    handleResponse(&response);
                }
#endif
        }
    }

    uint16_t width, height;
    mgpu_display_get_dimensions(display, &width, &height);
    frameBuffer = mgpu_framebuffer_new(width, height, operation.initialize.frameBufferScale, &standardAllocator);
    if (frameBuffer == NULL) {
        ESP_LOGE(LOG_TAG, "Framebuffer could not be created");
        return false;
    }

    ESP_LOGI(LOG_TAG, "Initialization successful");
    return true;
}

void app_main(void) {
    ESP_LOGI(LOG_TAG, "Starting Microgpu");
    if (!setup()) {
        ESP_LOGE(LOG_TAG, "Setup failed, exiting");
        return;
    }

    if (!wait_for_initialization()) {
        ESP_LOGE(LOG_TAG, "No initialization occurred, exiting");
        return;
    }

    Mgpu_Operation operation;
    while (1) {
        if (resetRequested) {
            ESP_LOGW(LOG_TAG, "Reset requested, exiting");
            return;
        }

        if (mgpu_databus_get_next_operation(databus, &operation)) {
            Mgpu_FrameBuffer *releasedFrameBuffer = NULL;
            mgpu_execute_operation(&operation, frameBuffer, display, databus, &resetRequested, &releasedFrameBuffer);
#ifdef DATABUS_TEST
            Mgpu_Response response;
            if (mgpu_test_databus_get_last_response(databus, &response)) {
                handleResponse(&response);
            }
#endif
        }
    }
}
