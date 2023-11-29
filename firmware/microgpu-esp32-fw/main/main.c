#include <stdbool.h>
#include <string.h>
#include "esp_log.h"
#include "microgpu-common/operations/execution//drawing/triangle.h"
#include "microgpu-common/messages.h"
#include "microgpu-common/alloc.h"
#include "microgpu-common/databus.h"
#include "microgpu-common/display.h"
#include "microgpu-common/operations/operation_execution.h"
#include "common.h"
#include "displays/i80_display.h"

#if defined(CONFIG_MICROGPU_DATABUS_SPI)
#include "spi_databus.h"
#elif defined(CONFIG_MICROGPU_DATABUS_BENCHMARK)
#include "benchmark_databus.h"
#elif
#error "No databus defined"
#endif

Mgpu_Display *display;
Mgpu_DisplayOptions displayOptions;
Mgpu_Databus *databus;
Mgpu_DatabusOptions databusOptions;
Mgpu_TextureManager *textureManager;
bool resetRequested;

static const Mgpu_Allocator standardAllocator = {
        .AllocateFn = malloc,
        .FreeFn = free,
};

bool setup(void) {
    ESP_LOGI(LOG_TAG, "Initializing display");

    // TODO: Make most of this settable in config options
    displayOptions.pixelWidth = CONFIG_MICROGPU_DISPLAY_WIDTH;
    displayOptions.pixelHeight = CONFIG_MICROGPU_DISPLAY_HEIGHT;
    displayOptions.controlPins.reset = CONFIG_MICROGPU_DISPLAY_RESET_PIN;
    displayOptions.controlPins.chipSelect = CONFIG_MICROGPU_DISPLAY_CS_PIN;
    displayOptions.controlPins.dataCommand = CONFIG_MICROGPU_DISPLAY_DC_PIN;
    displayOptions.controlPins.writeClock = CONFIG_MICROGPU_DISPLAY_WR_PIN;
    displayOptions.dataPins.data0 = CONFIG_MICROGPU_DISPLAY_DATA_0;
    displayOptions.dataPins.data1 = CONFIG_MICROGPU_DISPLAY_DATA_1;
    displayOptions.dataPins.data2 = CONFIG_MICROGPU_DISPLAY_DATA_2;
    displayOptions.dataPins.data3 = CONFIG_MICROGPU_DISPLAY_DATA_3;
    displayOptions.dataPins.data4 = CONFIG_MICROGPU_DISPLAY_DATA_4;
    displayOptions.dataPins.data5 = CONFIG_MICROGPU_DISPLAY_DATA_5;
    displayOptions.dataPins.data6 = CONFIG_MICROGPU_DISPLAY_DATA_6;
    displayOptions.dataPins.data7 = CONFIG_MICROGPU_DISPLAY_DATA_7;

    display = mgpu_display_new(&standardAllocator, &displayOptions);
    if (display == NULL) {
        ESP_LOGE(LOG_TAG, "Display could not be created");
        return false;
    }

    init_databus_options(&databusOptions);
    databus = mgpu_databus_new(&databusOptions, &standardAllocator);
    if (databus == NULL) {
        ESP_LOGE(LOG_TAG, "Databus could not be created");
        return false;
    }

    textureManager = mgpu_texture_manager_new(&standardAllocator);
    if (textureManager == NULL) {
        ESP_LOGE(LOG_TAG, "Texture manager could not be created");
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
            mgpu_execute_operation(&operation, display, databus, &resetRequested, textureManager);

            char *currentMessage = mgpu_message_get_pointer();
            if (currentMessage != NULL && strlen(currentMessage) > 0) {
                ESP_LOGI(LOG_TAG, "Message from operation: %s", currentMessage);
            }
        }
    }

    uint16_t width, height;
    mgpu_display_get_dimensions(display, &width, &height);

    // Create the active frame buffer
    Mgpu_TextureDefinition frameBufferSpecs = {
            .width = width,
            .height = height,
            .id = 0,
            .transparentColor = mgpu_color_from_rgb888(0, 0, 0),
    };

    if (!mgpu_texture_define(textureManager, &frameBufferSpecs, operation.initialize.frameBufferScale)) {
        ESP_LOGE(LOG_TAG, "Framebuffer could not be created");
        return false;
    }

    ESP_LOGI(LOG_TAG, "Initialization successful");
    return true;
}

void app_main(void) {
    ESP_LOGI(LOG_TAG, "Starting Microgpu");
    ESP_LOGI(LOG_TAG, "Version: %s", MGPU_VERSION);
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
            mgpu_execute_operation(&operation, display, databus, &resetRequested, textureManager);
        }

        char *currentMessage = mgpu_message_get_pointer();
        if (currentMessage != NULL && strlen(currentMessage) > 0) {
            ESP_LOGI(LOG_TAG, "Message from operation: %s", currentMessage);
        }
    }
}
