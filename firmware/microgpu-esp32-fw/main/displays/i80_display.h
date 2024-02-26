#pragma once

/*
 * The i80 display allows using an intel 8080 parallel interfaced LCD (such as the ili9341).
 */

#include <esp_lcd_types.h>

/*
 * Structure containing which gpio pins are used for control actions
 */
typedef struct {
    int reset;
    int chipSelect;
    int dataCommand;
    int writeClock;
} Mgpu_I80ControlPins;

/*
 * First 8 data pins for the parallel interface
 */
typedef struct {
    int data0, data1, data2, data3, data4, data5, data6, data7;
} Mgpu_I80First8DataPins;

struct Mgpu_DisplayOptions {
    Mgpu_I80ControlPins controlPins;
    Mgpu_I80First8DataPins dataPins;
    uint16_t pixelWidth, pixelHeight;
};

struct Mgpu_Display {
    const Mgpu_Allocator *allocator;
    esp_lcd_panel_handle_t panel;
    uint16_t pixelWidth, pixelHeight;
    uint16_t *buffer1;
    uint16_t *buffer2;
    int linesPerBuffer;
};

void init_display_options(Mgpu_DisplayOptions *displayOptions);
