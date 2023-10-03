#pragma once

#include "microgpu-common/texture_manager.h"
#include "microgpu-common/databus.h"
#include "microgpu-common/operations.h"

void mgpu_exec_batch(Mgpu_BatchOperation *batchOperation,
                     Mgpu_FrameBuffer *frameBuffer,
                     Mgpu_Display *display,
                     Mgpu_Databus *databus,
                     bool *resetFlag,
                     Mgpu_FrameBuffer **releasedFrameBuffer,
                     Mgpu_TextureManager *textureManager);

