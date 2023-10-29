#pragma once

#include <assert.h>
#include "alloc.h"
#include "texture_manager.h"

/*
 * The definition of a microgpu display.
 */
typedef struct Mgpu_Display Mgpu_Display;

/*
 * Implementation specific structure that provides options for
 * configuring displays.
 */
typedef struct Mgpu_DisplayOptions Mgpu_DisplayOptions;

/*
 * Creates a new display instance. Display instances will keep a reference
 * to the allocator to ensure it uses the corresponding and correct free
 * function for any de-allocations.
 */
Mgpu_Display *mgpu_display_new(const Mgpu_Allocator *allocator, const Mgpu_DisplayOptions *options);

/*
 * De-initializes the display and frees memory the display had allocated
 * to it. The pointer to the display will also be freed, and consumers should
 * not refer to that address anymore.
 */
void mgpu_display_free(Mgpu_Display *display);

/*
 * Gets the number of horizontal and vertical pixelBytes on the display being used.
 */
void mgpu_display_get_dimensions(Mgpu_Display *display, uint16_t *width, uint16_t *height);

/*
 * Renders the contents of the currently active frame buffer to the display. The framebuffer passed into
 * the display with this call should *not* be written to while the display has control
 * of it, which can last long beyond the return of this render function call.
 *
 * The display *may* swap texture 0 to another texture id if the display has to hold onto the current
 * framebuffer to continuously feed the display, but the texture it replaces texture id 0 with should
 * have the same dimensions and scale.
 */
void mgpu_display_render(Mgpu_Display *display, Mgpu_TextureManager *textureManager);
