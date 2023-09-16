#pragma once

#include <assert.h>
#include "alloc.h"
#include "framebuffer.h"

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
 * Gets the number of horizontal and vertical pixels on the display being used.
 */
void mgpu_display_get_dimensions(Mgpu_Display *display, uint16_t *width, uint16_t *height);

/*
 * Renders the contents of the frame buffer to the display. The framebuffer passed into
 * the display with this call should *not* be written to while the display has control
 * of it, which can last long beyond the return of this render function call.
 *
 * While some displays can have the frame buffer set directly to the display once and
 * be done (e.g. ili9341), other displays do not have built in frame buffers, and thus
 * the microgpu firmware controlling the display will be required to constantly feed
 * pixels every refresh cycle to the display.
 *
 * Mutating the framebuffer while the display is using it can cause flickering and partial
 * pixel reads at a minimum.
 *
 * The display releases control of the framebuffer by returning a pointer to the framebuffer
 * it is releasing as the return value of this render call. If the display does not yet have
 * a framebuffer to release, then a `NULL` pointer will be return, signalling to the firmware
 * that it should create a new framebuffer to do the next operations in to support double
 * buffering.
 *
 * If a display does not need to retain control of a framebuffer longer than the render
 * function call's lifetime, then it will return a pointer to the same framebuffer that
 * was passed in as an argument to it.
 *
 */
Mgpu_FrameBuffer *mgpu_display_render(Mgpu_Display *display, Mgpu_FrameBuffer *frameBuffer);
