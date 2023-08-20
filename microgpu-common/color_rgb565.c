#include "color.h"

Mgpu_Color mgpu_color_from_rgb565(uint8_t red, uint8_t green, uint8_t blue) {
    Mgpu_Color color = {
            .red = red,
            .green = green,
            .blue = blue,
    };

    return color;
}

void mgpu_color_get_rgb565(Mgpu_Color color, uint8_t *red, uint8_t *green, uint8_t *blue) {
    *red = color.red;
    *green = color.green;
    *blue = color.blue;
}

Mgpu_Color mgpu_color_from_rgb888(uint8_t red, uint8_t green, uint8_t blue) {
    // Convert values from 24bit space values to 16bit space values, before
    // packing them into a 16bit rgb565 number
    red = red / 8;
    green = green / 4;
    blue = blue / 8;

    return mgpu_color_from_rgb565(red, green, blue);
}

void mgpu_color_get_rgb888(Mgpu_Color color, uint8_t *red, uint8_t *green, uint8_t *blue) {
    mgpu_color_get_rgb565(color, red, green, blue);

    uint16_t tempRed = (uint16_t)(*red) * 8;
    uint16_t tempGreen = (uint16_t)(*green) * 4;
    uint16_t tempBlue = (uint16_t)(*green) * 8;

    if (tempRed > 255) {
        tempRed = 255;
    }

    if (tempBlue > 255) {
        tempBlue = 255;
    }

    if (tempGreen > 255) {
        tempGreen = 255;
    }

    *red = tempRed;
    *green = tempGreen;
    *blue = tempBlue;
}
