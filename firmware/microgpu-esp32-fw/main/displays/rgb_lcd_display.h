#pragma once
#include <esp_lcd_types.h>
#include "microgpu-common/alloc.h"

typedef struct {
    int data0, data1, data2, data3, data4, data5, data6, data7,
        data8, data9, data10, data11, data12, data13, data14, data15;
} Mgpu_Rgb16BitDataPins;

typedef struct {
    int pixelClock, vsync, hsync, de, backlight;
} Mgpu_RgbControlPins;

struct Mgpu_DisplayOptions {
    Mgpu_RgbControlPins controlPins;
    Mgpu_Rgb16BitDataPins dataPins;
    uint16_t pixelWidth, pixelHeight;
};

struct Mgpu_Display {
    const Mgpu_Allocator *allocator;
    esp_lcd_panel_handle_t panel;
    uint16_t pixelWidth, pixelHeight;
};

void init_display_options(Mgpu_DisplayOptions *options);
