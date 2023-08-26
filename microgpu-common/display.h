#pragma once

#include <assert.h>
#include "alloc.h"
#include "framebuffer.h"

/*
 * The definition of a microgpu display.
 */
typedef struct Mgpu_Display Mgpu_Display;

/*
 * Creates a new display instance. Display instances will keep a reference
 * to the allocator to ensure it uses the corresponding and correct free
 * function for any deallocations.
 */
Mgpu_Display *mgpu_display_new(const Mgpu_Allocator *allocator);

/*
 * De-initializes the display and frees memory the display had allocated
 * to it. The pointer to the display will also be freed, and consumers should
 * not refer to that address anymore.
 */
void mgpu_display_free(Mgpu_Display *display);

/*
 * Gets the number of horizontal and vertical pixels on the display being used.
 */
void mgpu_display_get_dimensions(Mgpu_Display *display, uint16_t *width, uint16_t *height);

/*
 * Renders the contents of the frame buffer to the display
 */
void mgpu_display_render(Mgpu_Display *display, Mgpu_FrameBuffer *frameBuffer);
