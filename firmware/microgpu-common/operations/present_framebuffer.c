#include <assert.h>
#include "present_framebuffer.h"

void mgpu_exec_present_framebuffer(Mgpu_Display *display,
                                   Mgpu_FrameBuffer *frameBuffer,
                                   Mgpu_FrameBuffer **releasedFrameBuffer) {
    assert(display != NULL);
    assert(frameBuffer != NULL);
    assert(releasedFrameBuffer != NULL);

    *releasedFrameBuffer = mgpu_display_render(display, frameBuffer);

    if (*releasedFrameBuffer != NULL) {
        // Clear the released frame buffer, so it's ready for the next drawing commands.
        // We don't want to allow drawing to a previously filled in frame buffer, because
        // if the display is still holding onto the current frame's frame buffer then the
        // released frame buffer was for the previous frame, and thus is inaccurate in
        // hard to predict ways.
        //
        // If persisting drawing is desired then re-playing previous draw commands is
        // probably the best answer.
        Mgpu_Color color = mgpu_color_from_rgb888(0, 0, 0);
        mgpu_framebuffer_clear(*releasedFrameBuffer, color);
    }
}
