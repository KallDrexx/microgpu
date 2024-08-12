#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <microgpu-common/common.h>

#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "microgpu-common/operations/execution//drawing/triangle.h"
#include "microgpu-common/messages.h"
#include "microgpu-common/alloc.h"
#include "microgpu-common/databus.h"
#include "microgpu-common/display.h"
#include "microgpu-common/fonts/fonts.h"
#include "microgpu-common/operations/operation_execution.h"
#include "common.h"

#if defined(CONFIG_MICROGPU_DATABUS_SPI)
#include "spi_databus.h"
#elif defined(CONFIG_MICROGPU_DATABUS_BENCHMARK)
#include "benchmark_databus.h"
#else
#error "No databus defined"
#endif

#if defined(CONFIG_MICROGPU_DISPLAY_16BIT_RGB_LCD)
#include "displays/rgb_lcd_display.h"
#elif defined(CONFIG_MICROGPU_DISPLAY_I80_8BIT_ILI9341)
#include "displays/i80_display.h"
#else
#error "No display defined"
#endif

Mgpu_Display *display;
Mgpu_DisplayOptions displayOptions;
Mgpu_Databus *databus;
Mgpu_DatabusOptions databusOptions;
Mgpu_TextureManager *textureManager;
bool resetRequested;

void *alloc_internal_ram(size_t size);

void *alloc_spi_ram(size_t size);

static const Mgpu_Allocator standardAllocator = {
        .FastMemAllocateFn = alloc_internal_ram,
        .FastMemFreeFn = free,
        .SlowMemAllocateFn = alloc_spi_ram,
        .SlowMemFreeFn = free,
};

bool setup(void) {
    ESP_LOGI(LOG_TAG, "Initializing display");
    init_display_options(&displayOptions);

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

bool define_display_framebuffer(uint8_t scale) {
    uint16_t width, height;
    mgpu_display_get_dimensions(display, &width, &height);

    // Create the active frame buffer
    Mgpu_TextureDefinition frameBufferSpecs = {
            .width = width,
            .height = height,
            .id = 0,
            .transparentColor = mgpu_color_from_rgb888(0, 0, 0),
            .flags = 0, // allocate in fast ram
    };

    if (!mgpu_texture_define(textureManager, &frameBufferSpecs, scale)) {
        ESP_LOGE(LOG_TAG, "Framebuffer could not be created");
        return false;
    }

    return true;
}

bool show_boot_screen(void) {
    if (!define_display_framebuffer(1)) {
        return false;
    }

    char versionString[200] = {0};
    char apiString[200] = {0};
    snprintf(versionString, sizeof(versionString), "Firmware version: %s", MGPU_VERSION);
    sniprintf(apiString, sizeof(apiString), "API version: %u", MGPU_API_VERSION);

    Mgpu_Color white = mgpu_color_from_rgb888(255, 255, 255);
    mgpu_font_draw(textureManager, Mgpu_Font_Font8x12, 0, "Microgpu", white, 10, 10);
    mgpu_font_draw(textureManager, Mgpu_Font_Font8x12, 0, versionString, white, 10, 25);
    mgpu_font_draw(textureManager, Mgpu_Font_Font8x12, 0, apiString, white, 10, 40);

    uint16_t width, height;
    mgpu_display_get_dimensions(display, &width, &height);

    uint16_t startY = height - 25;
    mgpu_font_draw(textureManager, Mgpu_Font_Font8x12, 0, "Waiting for Initialization...", white, 10, startY);

    mgpu_display_render(display, textureManager);

    // Now that the boot screen is rendered, we can free the frame buffer we were currently using, as we won't
    // write to it or render it again. This is also a small hack to keep the get status operation as returning
    // that it's not initialized yet, since "is the GPU initialized" is based on a framebuffer texture existing.
    Mgpu_TextureDefinition clearTexture = {
        .id = 0,
        .height = 0,
        .width = 0,
        .flags = 0, // fast ram
    };
    mgpu_texture_define(textureManager, &clearTexture, 1);

    return true;
}

bool wait_for_initialization(void) {
    ESP_LOGI(LOG_TAG, "Waiting for initialization operation");
    ESP_LOGI(LOG_TAG, "API version: %u", MGPU_API_VERSION);
    if (!show_boot_screen()) {
        return false;
    }

    Mgpu_Operation operation;

    while (true) {
        memset(&operation, 0, sizeof(Mgpu_Operation));
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

    if (!define_display_framebuffer(operation.initialize.frameBufferScale)) {
        return false;
    }

    ESP_LOGI(LOG_TAG, "Initialization successful");
    return true;
}

void memset_16(uint8_t *destination, uint16_t value, size_t count) {
    uint32_t value32 = (uint32_t) value | ((uint32_t) value << 16);
    uint32_t *dest32 = (uint32_t*) destination;

    // Set 4 bytes at a time
    for (; count >= 2; count -= 2) {
        *dest32++ = value32;
    }

    if (count) {
        *(uint16_t*)dest32 = value;
    }
}

void transparent_test(uint16_t *destination, uint16_t *source, size_t count) {
    size_t counter = 0;
    bool isTransparent = false;
    for (; count > 0; count--) {
        if (counter >= 5) {
            counter = 0;
            isTransparent = !isTransparent;
        }

        if (!isTransparent) {
            *destination = *source;
        }

        destination++;
        source++;
        counter++;
    }
}

size_t rle_encode(uint8_t *buffer, size_t length) {
    uint8_t transparentCount = 5;
    uint8_t pixelCount = 50;
    size_t index = 0;
    while (true) {
        size_t bytesNeeded = (pixelCount * 2) + 2;
        if (index + bytesNeeded >= length) {
            return index + 1;
        }

        buffer[index] = transparentCount;
        buffer[index + 1] = pixelCount;
        for (size_t x = 0; x < pixelCount; x++) {
            buffer[index + 2 + (x * 2)] = 0xa5;
            buffer[index + 2 + (x * 2) + 1] = 0x5a;
        }

        index += bytesNeeded;
    }
}

void rle_draw(uint16_t *destination, uint8_t *source, size_t sourceCount) {
    uint8_t *sourceEnd = source + sourceCount;

    while (source < sourceEnd) {
        size_t transparentCount = *source;
        destination += transparentCount;

        size_t pixelCount = *(source + 1);
        source += 2;

        memcpy(destination, source, pixelCount * 2);
        source += pixelCount * 2;
        destination += pixelCount;
    }
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

    #define TEST_SIZE (800 * 8)
    uint8_t *testBuffer = heap_caps_malloc(TEST_SIZE * sizeof(Mgpu_Color), MALLOC_CAP_DMA);
    uint8_t *testBuffer2 = heap_caps_malloc(TEST_SIZE * sizeof(Mgpu_Color), MALLOC_CAP_32BIT);
    assert(testBuffer != NULL);
    assert(testBuffer2 != NULL);
    memset_16(testBuffer2, 0xabba, TEST_SIZE);

    uint8_t *rleBuffer = heap_caps_malloc(TEST_SIZE * sizeof(Mgpu_Color), MALLOC_CAP_32BIT);
    assert(rleBuffer != NULL);
    memset(rleBuffer, 0, TEST_SIZE * sizeof(Mgpu_Color));
    size_t rleBytes = rle_encode(rleBuffer, 256 * sizeof(Mgpu_Color));

    Mgpu_Operation operation;
    while (1) {
        int64_t start = esp_timer_get_time();
//        memset(testBuffer, 0xA55A, 800 * sizeof(Mgpu_Color));
        memset_16(testBuffer + 800, 0xA55A, 800);
        int64_t end = esp_timer_get_time();
        ESP_LOGI(LOG_TAG, "Memset time: %lld", end - start);

        start = esp_timer_get_time();
        memcpy(testBuffer + 800, testBuffer2, 800 * sizeof(Mgpu_Color));
        end = esp_timer_get_time();
        ESP_LOGI(LOG_TAG, "Memcpy time: %lld", end - start);

        start = esp_timer_get_time();
        transparent_test(((uint16_t*)testBuffer) + 800, (uint16_t*)testBuffer2, 800);
        end = esp_timer_get_time();
        ESP_LOGI(LOG_TAG, "Transparency time: %lld", end - start);

        for (int x = 0; x < 10; x++) {
            memset(testBuffer, 0, TEST_SIZE * sizeof(Mgpu_Color));
            start = esp_timer_get_time();
            rle_draw((uint16_t *) testBuffer, rleBuffer, rleBytes);
            end = esp_timer_get_time();
            ESP_LOGI(LOG_TAG, "RLE time: %lld", end - start);
        }

//        printf("Value: ");
//        for (int x = 0; x < 800 * 2; x++) {
//            printf("%02X ", testBuffer[x]);
//        }
//        printf("\n");

        if (resetRequested) {
            ESP_LOGW(LOG_TAG, "Reset requested, exiting");
            return;
        }

        memset(&operation, 0, sizeof(Mgpu_Operation));
        if (mgpu_databus_get_next_operation(databus, &operation)) {
            mgpu_execute_operation(&operation, display, databus, &resetRequested, textureManager);
        }

        char *currentMessage = mgpu_message_get_pointer();
        if (currentMessage != NULL && strlen(currentMessage) > 0) {
            ESP_LOGI(LOG_TAG, "Message from operation: %s", currentMessage);
        }
    }
}

void *alloc_internal_ram(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_32BIT);
}

void *alloc_spi_ram(size_t size) {
    #ifdef CONFIG_SPIRAM
        return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
    #else
        return heap_caps_malloc(size, MALLOC_CAP_32BIT);
    #endif
}
