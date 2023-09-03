#pragma once

#include "microgpu-common/display.h"
#include "microgpu-common/framebuffer.h"

void mgpu_exec_present_framebuffer(Mgpu_Display *display,
                                   Mgpu_FrameBuffer *frameBuffer,
                                   Mgpu_FrameBuffer **releasedFrameBuffer);
