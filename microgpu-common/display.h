#pragma once

#include <assert.h>
#include "framebuffer.h"

/*
 * The definition of a microgpu display.
 */
typedef struct Mgpu_Display Mgpu_Display;

/*
 * Gets the amount of memory that needs to be allocated to create the display
 */
size_t mgpu_display_get_size();

/*
 * Creates a newly initialized display using the passed in memory. The memory pointer passed
 * in must be pre-allocated to the same amount of memory returned in the
 * `mgpu_display_get_size()` call.
 *
 * Will return a NULL pointer if initialization fails.
 */
Mgpu_Display *mgpu_display_init(void *memory);

/*
 * Tears down the display.
 *
 * The caller is responsible for freeing the memory used by the
 * display pointer itself (the same memory that was passed into
 * `mgpu_display_init()`.
 */
void mgpu_display_uninit(Mgpu_Display *display);

/*
 * Gets the number of horizontal and vertical pixels on the display being used.
 */
void mgpu_display_get_dimensions(Mgpu_Display *display, uint16_t *width, uint16_t *height);

/*
 * Renders the contents of the frame buffer to the display
 */
void mgpu_display_render(Mgpu_Display *display, Mgpu_FrameBuffer *frameBuffer);
