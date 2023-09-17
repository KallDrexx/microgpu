#include <stdbool.h>
#include <esp_heap_caps.h>
#include "esp_log.h"
#include "microgpu-common/alloc.h"
#include "microgpu-common/display.h"
#include "microgpu-common/framebuffer.h"
#include "common.h"
#include "displays/i80_display.h"

Mgpu_FrameBuffer *frameBuffer;
Mgpu_Display *display;
Mgpu_DisplayOptions displayOptions;

void *alloc_external_ram(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}

//static const Mgpu_Allocator psramAllocator = {
//        .AllocateFn = alloc_external_ram,
//        .FreeFn = free,
//};

static const Mgpu_Allocator standardAllocator = {
        .AllocateFn = malloc,
        .FreeFn = free,
};

void setup_display_options() {
    // TODO: Make most of this settable in config options
    displayOptions.pixelWidth = 240;
    displayOptions.pixelHeight = 320;
    displayOptions.controlPins.reset = 2;
    displayOptions.controlPins.chipSelect = 3;
    displayOptions.controlPins.dataCommand = 4;
    displayOptions.controlPins.writeClock = 5;
    displayOptions.dataPins.data0 = 6;
    displayOptions.dataPins.data0 = 7;
    displayOptions.dataPins.data0 = 8;
    displayOptions.dataPins.data0 = 9;
    displayOptions.dataPins.data0 = 10;
    displayOptions.dataPins.data0 = 11;
    displayOptions.dataPins.data0 = 12;
    displayOptions.dataPins.data0 = 13;
}

bool setup(void) {
    setup_display_options();
    display = mgpu_display_new(&standardAllocator, &displayOptions);
    if (display == NULL) {
        ESP_LOGE(LOG_TAG, "No display was created");
        return false;
    }

    return true;
}

void app_main(void)
{
    if (!setup()) {
        ESP_LOGE(LOG_TAG, "Setup failed, exiting");
        return;
    }


}
