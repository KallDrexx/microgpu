#include "color.h"

Mgpu_ColorMode mgpu_color_get_mode(void) {
    return MgpuColorMode_Rgb565;
}

Mgpu_Color mgpu_color_from_rgb565(uint8_t red, uint8_t green, uint8_t blue) {
    return ((uint16_t) red << 11) | ((uint16_t) green << 5) | blue;
}

void mgpu_color_get_rgb565(Mgpu_Color color, uint8_t *red, uint8_t *green, uint8_t *blue) {
    *red = (color >> 11);
    *green = ((color & 0x0730) >> 5);
    *blue = color & 0x001f;
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
    uint16_t tempRed = (color >> 11) * 8;
    uint16_t tempGreen = ((color & 0x0730) >> 5) * 4;
    uint16_t tempBlue = (color & 0x1F) * 8;

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

Mgpu_Color mgpu_color_deserialize(const uint8_t bytes[], size_t firstColorByteIndex, size_t *nextIndex) {
    uint8_t red = (bytes[firstColorByteIndex] & 0xF8) >> 3;
    uint8_t green = (bytes[firstColorByteIndex] & 0x07) << 3 | (bytes[firstColorByteIndex + 1] & 0xE0) >> 5;
    uint8_t blue = bytes[firstColorByteIndex + 1] & 0x1F;

    *nextIndex = firstColorByteIndex + 2;
    return mgpu_color_from_rgb565(red, green, blue);
}

size_t mgpu_color_bytes_per_pixel() {
    return 2;
}