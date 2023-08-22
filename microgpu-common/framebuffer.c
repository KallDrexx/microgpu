#include <assert.h>
#include "framebuffer.h"

size_t mgpu_framebuffer_get_required_buffer_size(uint16_t width, uint16_t height) {
    return width * height * sizeof(Mgpu_Color);
}

Mgpu_FrameBuffer mgpu_framebuffer_init(void *memory, uint16_t width, uint16_t height, uint8_t scale) {
    assert(memory != NULL);

    Mgpu_FrameBuffer framebuffer = {
            .pixels = (Mgpu_Color *)memory,
            .height = height,
            .width = width,
            .scale = scale,
    };

    for (size_t index = 0; index < width * height; index++) {
        framebuffer.pixels[index] = mgpu_color_from_rgb888(0, 0, 0);
    }

    return framebuffer;
}
