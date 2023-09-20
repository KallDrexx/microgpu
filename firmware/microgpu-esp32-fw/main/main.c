#include <stdbool.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "microgpu-common/alloc.h"
#include "microgpu-common/display.h"
#include "microgpu-common/framebuffer.h"
#include "microgpu-common/operations/drawing/rectangle.h"
#include "microgpu-common/operations/drawing/triangle.h"
#include "common.h"
#include "displays/i80_display.h"
#include "spi_databus.h"

Mgpu_FrameBuffer *frameBuffer;
Mgpu_Display *display;
Mgpu_DisplayOptions displayOptions;

static const Mgpu_Allocator standardAllocator = {
        .AllocateFn = malloc,
        .FreeFn = free,
};

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
        ESP_LOGE(LOG_TAG, "No display was created");
        return false;
    }

    return true;
}

void app_main(void)
{
    ESP_LOGI(LOG_TAG, "Starting Microgpu");
    if (!setup()) {
        ESP_LOGE(LOG_TAG, "Setup failed, exiting");
        return;
    }

    uint16_t width, height;
    mgpu_display_get_dimensions(display, &width, &height);

    frameBuffer = mgpu_framebuffer_new(width, height, 1, &standardAllocator);

    Mgpu_DrawRectangleOperation rectangle = {
            .width = 200,
            .height = 100,
            .startX = 10,
            .startY = 100,
            .color = mgpu_color_from_rgb888(255, 0, 0),
    };

    Mgpu_DrawTriangleOperation triangle = {
            .x0 = 25,
            .y0 = 10,
            .x1 = 45,
            .y1 = 100,
            .x2 = 300,
            .y2 = 200,
            .color = mgpu_color_from_rgb888(0, 255, 0),
    };

    mgpu_draw_rectangle(&rectangle, frameBuffer);
    mgpu_draw_triangle(&triangle, frameBuffer);
    mgpu_display_render(display, frameBuffer);

    while(1) {
        vTaskDelay(1000);
    }
}
