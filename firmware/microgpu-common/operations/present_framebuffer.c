#include <assert.h>
#include "present_framebuffer.h"

void mgpu_exec_present_framebuffer(Mgpu_Display *display,
                                   Mgpu_FrameBuffer *frameBuffer,
                                   Mgpu_FrameBuffer **releasedFrameBuffer) {
    assert(display != NULL);
    assert(frameBuffer != NULL);
    assert(releasedFrameBuffer != NULL);

    *releasedFrameBuffer = mgpu_display_render(display, frameBuffer);
}
