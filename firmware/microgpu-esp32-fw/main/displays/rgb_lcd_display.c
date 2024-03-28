#include <esp_lcd_panel_rgb.h>
#include <esp_log.h>
#include <esp_lcd_panel_ops.h>
#include <driver/gpio.h>
#include "microgpu-common/display.h"
#include "rgb_lcd_display.h"
#include "common.h"

void log_display_options(const Mgpu_DisplayOptions *options) {
    ESP_LOGI(LOG_TAG, "Microgpu RGB LCD display options:");
    ESP_LOGI(LOG_TAG, "Display resolution: %u x %u", options->pixelWidth, options->pixelHeight);
    ESP_LOGI(LOG_TAG, "Data0 pin: %u", options->dataPins.data0);
    ESP_LOGI(LOG_TAG, "Data1 pin: %u", options->dataPins.data1);
    ESP_LOGI(LOG_TAG, "Data2 pin: %u", options->dataPins.data2);
    ESP_LOGI(LOG_TAG, "Data3 pin: %u", options->dataPins.data3);
    ESP_LOGI(LOG_TAG, "Data4 pin: %u", options->dataPins.data4);
    ESP_LOGI(LOG_TAG, "Data5 pin: %u", options->dataPins.data5);
    ESP_LOGI(LOG_TAG, "Data6 pin: %u", options->dataPins.data6);
    ESP_LOGI(LOG_TAG, "Data7 pin: %u", options->dataPins.data7);
    ESP_LOGI(LOG_TAG, "Data7 pin: %u", options->dataPins.data8);
    ESP_LOGI(LOG_TAG, "Data7 pin: %u", options->dataPins.data9);
    ESP_LOGI(LOG_TAG, "Data7 pin: %u", options->dataPins.data10);
    ESP_LOGI(LOG_TAG, "Data7 pin: %u", options->dataPins.data11);
    ESP_LOGI(LOG_TAG, "Data7 pin: %u", options->dataPins.data12);
    ESP_LOGI(LOG_TAG, "Data7 pin: %u", options->dataPins.data13);
    ESP_LOGI(LOG_TAG, "Data7 pin: %u", options->dataPins.data14);
    ESP_LOGI(LOG_TAG, "Data7 pin: %u", options->dataPins.data15);

    ESP_LOGI(LOG_TAG, "pclk pin: %u", options->controlPins.pixelClock);
    ESP_LOGI(LOG_TAG, "vsync pin: %u", options->controlPins.vsync);
    ESP_LOGI(LOG_TAG, "hsync pin: %u", options->controlPins.hsync);
    ESP_LOGI(LOG_TAG, "de pin: %u", options->controlPins.de);
    ESP_LOGI(LOG_TAG, "backlight pin: %u", options->controlPins.backlight);
}

void init_lcd(const Mgpu_DisplayOptions *options, esp_lcd_panel_handle_t *handle) {
    log_display_options(options);
    esp_lcd_rgb_panel_config_t panel_config = {
            .data_width = 16,
            .psram_trans_align = 64,
            .num_fbs = 1,
            .bounce_buffer_size_px = 8 * options->pixelWidth,
            .clk_src = LCD_CLK_SRC_DEFAULT,
            .disp_gpio_num = -1,
            .pclk_gpio_num = options->controlPins.pixelClock,
            .vsync_gpio_num = options->controlPins.vsync,
            .hsync_gpio_num = options->controlPins.hsync,
            .de_gpio_num = options->controlPins.de,
            .data_gpio_nums = {
                    options->dataPins.data0,
                    options->dataPins.data1,
                    options->dataPins.data2,
                    options->dataPins.data3,
                    options->dataPins.data4,
                    options->dataPins.data5,
                    options->dataPins.data6,
                    options->dataPins.data7,
                    options->dataPins.data8,
                    options->dataPins.data9,
                    options->dataPins.data10,
                    options->dataPins.data11,
                    options->dataPins.data12,
                    options->dataPins.data13,
                    options->dataPins.data14,
                    options->dataPins.data15,
            },
            .timings = {
                    .pclk_hz = (CONFIG_MICROGPU_DISPLAY_PCLK_MHZ * 1000 * 1000),
                    .h_res = options->pixelWidth,
                    .v_res = options->pixelHeight,
                    // The following parameters should refer to LCD spec
                    .hsync_back_porch = 8,
                    .hsync_front_porch = 8,
                    .hsync_pulse_width = 4,
                    .vsync_back_porch = 8,
                    .vsync_front_porch = 8,
                    .vsync_pulse_width = 4,
                    .flags.pclk_active_neg = true,
                    .flags.hsync_idle_low = 1,
                    .flags.vsync_idle_low = 1,
            },
            .flags.fb_in_psram = true,
    };

    gpio_config_t bk_gpio_config = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = 1ULL << options->controlPins.backlight,
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&panel_config, handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(*handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(*handle));
    ESP_ERROR_CHECK(gpio_set_level(options->controlPins.backlight, 1));
//    esp_lcd_panel_invert_color(*handle, true);
}

Mgpu_Display *mgpu_display_new(const Mgpu_Allocator *allocator, const Mgpu_DisplayOptions *options) {
    assert(options != NULL);
    mgpu_alloc_assert(allocator);

    esp_lcd_panel_handle_t panel = NULL;
    init_lcd(options, &panel);

    Mgpu_Display *display = allocator->FastMemAllocateFn(sizeof(Mgpu_Display));
    if (display == NULL) {
        ESP_LOGE(LOG_TAG, "Failed to allocate space for display structure");
        return NULL;
    }

    display->allocator = allocator;
    display->panel = panel;
    display->pixelWidth = options->pixelWidth;
    display->pixelHeight = options->pixelHeight;

    return display;
}

void mgpu_display_free(Mgpu_Display *display) {
    if (display) {
        display->allocator->FastMemFreeFn(display);
    }
}

void mgpu_display_get_dimensions(Mgpu_Display *display, uint16_t *width, uint16_t *height) {
    assert(display != NULL);
    assert(width != NULL);
    assert(height != NULL);

    *width = display->pixelWidth;
    *height = display->pixelHeight;
}

void mgpu_display_render(Mgpu_Display *display, Mgpu_TextureManager *textureManager) {
    assert(display != NULL);
    assert(textureManager != NULL);

    Mgpu_Texture *frameBuffer = mgpu_texture_get(textureManager, 0);
    assert(frameBuffer != NULL);

    assert(frameBuffer->scale == 1); // TODO: Add support for scaling
    esp_lcd_panel_draw_bitmap(display->panel, 0, 0, display->pixelWidth, display->pixelHeight, frameBuffer->pixels);
}

void init_display_options(Mgpu_DisplayOptions *displayOptions) {
    assert(displayOptions != NULL);

    displayOptions->pixelWidth = CONFIG_MICROGPU_DISPLAY_WIDTH;
    displayOptions->pixelHeight = CONFIG_MICROGPU_DISPLAY_HEIGHT;
    displayOptions->controlPins.de = CONFIG_MICROGPU_DISPLAY_DE;
    displayOptions->controlPins.vsync = CONFIG_MICROGPU_DISPLAY_VSYNC;
    displayOptions->controlPins.hsync = CONFIG_MICROGPU_DISPLAY_HSYNC;
    displayOptions->controlPins.pixelClock = CONFIG_MICROGPU_DISPLAY_PIXEL_CLOCK_PIN;
    displayOptions->controlPins.backlight = CONFIG_MICROGPU_DISPLAY_BACKLIGHT;
    displayOptions->dataPins.data0 = CONFIG_MICROGPU_DISPLAY_DATA_0;
    displayOptions->dataPins.data1 = CONFIG_MICROGPU_DISPLAY_DATA_1;
    displayOptions->dataPins.data2 = CONFIG_MICROGPU_DISPLAY_DATA_2;
    displayOptions->dataPins.data3 = CONFIG_MICROGPU_DISPLAY_DATA_3;
    displayOptions->dataPins.data4 = CONFIG_MICROGPU_DISPLAY_DATA_4;
    displayOptions->dataPins.data5 = CONFIG_MICROGPU_DISPLAY_DATA_5;
    displayOptions->dataPins.data6 = CONFIG_MICROGPU_DISPLAY_DATA_6;
    displayOptions->dataPins.data7 = CONFIG_MICROGPU_DISPLAY_DATA_7;
    displayOptions->dataPins.data8 = CONFIG_MICROGPU_DISPLAY_DATA_8;
    displayOptions->dataPins.data9 = CONFIG_MICROGPU_DISPLAY_DATA_9;
    displayOptions->dataPins.data10 = CONFIG_MICROGPU_DISPLAY_DATA_10;
    displayOptions->dataPins.data11 = CONFIG_MICROGPU_DISPLAY_DATA_11;
    displayOptions->dataPins.data12 = CONFIG_MICROGPU_DISPLAY_DATA_12;
    displayOptions->dataPins.data13 = CONFIG_MICROGPU_DISPLAY_DATA_13;
    displayOptions->dataPins.data14 = CONFIG_MICROGPU_DISPLAY_DATA_14;
    displayOptions->dataPins.data15 = CONFIG_MICROGPU_DISPLAY_DATA_15;
}
