#pragma once

#include <stdint.h>

typedef enum {
    MgpuColorMode_Unspecified = 0,
    MgpuColorMode_Rgb565,
} Mgpu_ColorMode;

#ifdef MGPU_COLOR_MODE_USE_RGB565

typedef uint16_t Mgpu_Color;

/*
 * Creates a color based on red, green, and blue values already converted
 * to the RGB565 color space.
 */
Mgpu_Color mgpu_color_from_rgb565(uint8_t red, uint8_t green, uint8_t blue);

/*
 * Returns the red, green, and blue color values from an MGPU color represented
 * in the RGB565 color space values.
 */
void mgpu_color_get_rgb565(Mgpu_Color color, uint8_t *red, uint8_t *green, uint8_t *blue);

#endif // MGPU_COLOR_MODE_USE_RGB565

/*
 * Creates a color based on rgb values. Each value assumed to be between
 * 0 and 255, and will be converted to the proper color space as needed.
 */
Mgpu_Color mgpu_color_from_rgb24(uint8_t red, uint8_t green, uint8_t blue);

/*
 * Extracts the rgb 24 bit values from a color type.  Each color will be
 * between 0 and 255.
 */
void mgpu_color_get_rgb24(Mgpu_Color color, uint8_t *red, uint8_t *green, uint8_t *blue);
