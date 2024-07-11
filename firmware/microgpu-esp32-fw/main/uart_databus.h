#pragma once

#include <hal/uart_types.h>
#include "microgpu-common/alloc.h"

struct Mgpu_DatabusOptions {
    int baudRate, rxPin, txPin, uartNum, ctsPin;
};

struct Mgpu_Databus {
    const Mgpu_Allocator *allocator;
    int uartNum;
    uint8_t *sendPrepareBuffer, *receiveBuffer;
    size_t bytesInReceiveBuffer;
};

void init_databus_options(Mgpu_DatabusOptions *options);
