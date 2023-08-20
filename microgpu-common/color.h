#pragma once

#include <stdint.h>

typedef enum {
    MgpuColorMode_Unspecified = 0,
    MgpuColorMode_Rgb565,
} Mgpu_ColorMode;

#ifdef MGPU_COLOR_MODE_USE_RGB565

typedef struct {
    uint8_t red : 5;
    uint8_t green: 6;
    uint8_t blue: 5;
} Mgpu_Color;

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

#else

#error "No color mode specified"

#endif // MGPU_COLOR_MODE_USE_RGB565

/*
 * Gets the active color mode supported by the system
 */
Mgpu_ColorMode mgpu_color_get_mode(void);

/*
 * Creates a color based on rgb values. Each value assumed to be between
 * 0 and 255, and will be converted to the proper color space as needed.
 */
Mgpu_Color mgpu_color_from_rgb888(uint8_t red, uint8_t green, uint8_t blue);

/*
 * Extracts the rgb 24 bit values from a color type.  Each color will be
 * between 0 and 255.
 */
void mgpu_color_get_rgb888(Mgpu_Color color, uint8_t *red, uint8_t *green, uint8_t *blue);
