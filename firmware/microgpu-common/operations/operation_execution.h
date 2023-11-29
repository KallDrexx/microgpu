#pragma once

#include "microgpu-common/display.h"
#include "operations.h"
#include "microgpu-common/databus.h"
#include "microgpu-common/texture_manager.h"

/*
 * Attempts to execute the specified operation if supported. Most operations are supported, with
 * initialization being an exception.
 */
void mgpu_execute_operation(Mgpu_Operation *operation,
                            Mgpu_Display *display,
                            Mgpu_Databus *databus,
                            bool *resetFlag,
                            Mgpu_TextureManager *textureManager);
