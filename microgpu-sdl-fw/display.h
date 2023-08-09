#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "framebuffer.h"

typedef struct MGPU_SDL_Display_Internal MGPU_SDL_Display_internal;

/*
 * Structure that manages the application window and pushes pixels to the screen
 */
typedef struct {
    int windowWidth;
    int windowHeight;
    uint32_t *pixelBuffer;
    MGPU_SDL_Display_internal *internal;
} Mgpu_Sdl_Display;

/*
 * Initializes the display. Only a not-yet initialized display can be initialized.
 * The pixel buffer will be allocated by this function.
 */
bool mgpu_sdl_display_init(Mgpu_Sdl_Display *display);

/*
 * Uninitialized the display and deallocates any memory it's in charge of. The
 * display structure itself will not be deallocated and will need to be deallocated
 * by the caller (except for the pixel buffer field, which will be deallocated).
 */
void mgpu_sdl_display_uninit(Mgpu_Sdl_Display *display);

/*
 * Pushes the current pixel buffer to the SDL display
 */
void mgpu_sdl_push_to_screen(Mgpu_Sdl_Display *display);

/*
 * Sets the texture representing the image that will be displayed to the
 * pixel data from the frame buffer.
 */
void mgpu_sdl_apply_framebuffer(Mgpu_Sdl_Display *display, Mgpu_FrameBuffer *frameBuffer);
