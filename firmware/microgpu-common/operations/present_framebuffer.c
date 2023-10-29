#include <assert.h>
#include <vcruntime_string.h>
#include "present_framebuffer.h"

void mgpu_exec_present_framebuffer(Mgpu_Display *display, Mgpu_TextureManager *textureManager) {
    assert(display != NULL);
    assert(textureManager != NULL);

    mgpu_display_render(display, textureManager);

    // Clear the frame buffer, so it's ready for the next drawing commands.
    // We don't want to allow drawing to a previously filled in frame buffer, because
    // if the display is still holding onto the current frame's frame buffer then the
    // released frame buffer was for the previous frame, and thus is inaccurate in
    // hard to predict ways.
    //
    // If persisting drawing is desired then re-playing previous draw commands or
    // drawing to a cached texture is preferred.
    Mgpu_Texture *texture = mgpu_texture_get(textureManager, 0);
    assert(texture != NULL); // We should never not have an active frame buffer

    Mgpu_Color color = mgpu_color_from_rgb888(0, 0, 0);
    memset(texture->pixels, color, sizeof(Mgpu_Color) * texture->width * texture->height);
}
