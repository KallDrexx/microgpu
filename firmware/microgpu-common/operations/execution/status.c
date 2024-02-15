#include <microgpu-common/common.h>
#include "status.h"

void mgpu_exec_status_op(Mgpu_Display *display, Mgpu_TextureManager *textureManager, Mgpu_Databus *databus) {
    assert(display != NULL);
    assert(databus != NULL);
    assert(textureManager != NULL);

    uint16_t width, height;
    mgpu_display_get_dimensions(display, &width, &height);

    Mgpu_StatusResponse status = {
            .colorMode = mgpu_color_get_mode(),
            .displayHeight = height,
            .displayWidth = width,
            .opByteLimit = mgpu_databus_get_max_size(databus),
            .apiVersionId = MGPU_API_VERSION,
    };

    // We know we aren't initialized if we don't have a frame buffer yet
    Mgpu_Texture *frameBuffer = mgpu_texture_get(textureManager, 0);
    if (frameBuffer != NULL) {
        status.frameBufferWidth = frameBuffer->width;
        status.frameBufferHeight = frameBuffer->height;
        status.isInitialized = true;
    } else {
        status.frameBufferWidth = 0;
        status.frameBufferHeight = 0;
        status.isInitialized = false;
    }

    Mgpu_Response response = {
            .type = Mgpu_Response_Status,
            .status = status,
    };

    mgpu_databus_send_response(databus, &response);
}
