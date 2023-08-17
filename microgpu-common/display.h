#pragma once

#include <assert.h>
#include "framebuffer.h"

typedef struct Mgpu_Display Mgpu_Display;

/*
 * Alias for a pointer to a type of microgpu display interface. The actual value
 * pointed to must be castable to a known display type.
 */
typedef const void *Mgpu_DisplayPtr;

/*
 * A virtual table of function pointers that can be used to execute
 * functionality on a display.
 */
typedef struct {
    /*
     * Gets the width in pixelBuffer of the final output of the display
     */
    uint16_t (*get_width)(Mgpu_DisplayPtr);

    /*
     * Gets the height in pixelBuffer of the final output of the display
     */
    uint16_t (*get_height)(Mgpu_DisplayPtr);

    /*
     * Renders the supplied frame buffer to the display
     */
    void (*render_framebuffer)(Mgpu_DisplayPtr, Mgpu_FrameBuffer *);
} Mgpu_DisplayVTable;

/*
 * The definition of a microgpu display. The underlying memory location pointed
 * to by `self` must be castable to the type understood by the vtable.
 */
struct Mgpu_Display {
    Mgpu_DisplayPtr self;
    const Mgpu_DisplayVTable *vTable;
};

/*
 * Helper method to call the get_width vtable function
 */
static inline uint16_t mgpu_display_get_width(Mgpu_Display *display) {
    assert(display != NULL);
    assert(display->self != NULL);
    assert(display->vTable != NULL);

    return display->vTable->get_width(display->self);
}

/*
 * Helper method to call the get_height vtable function
 */
static inline uint16_t mgpu_display_get_height(Mgpu_Display *display) {
    assert(display != NULL);
    assert(display->self != NULL);
    assert(display->vTable != NULL);

    return display->vTable->get_height(display->self);
}

/*
 * Helper method to call the render_framebuffer vtable function
 */
static inline void mgpu_display_render_framebuffer(Mgpu_Display *display, Mgpu_FrameBuffer *frameBuffer) {
    assert(display != NULL);
    assert(display->self != NULL);
    assert(display->vTable != NULL);

    display->vTable->render_framebuffer(display->self, frameBuffer);
}
