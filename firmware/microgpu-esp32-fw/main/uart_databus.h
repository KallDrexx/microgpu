#pragma once

#include <hal/uart_types.h>
#include "microgpu-common/alloc.h"

struct Mgpu_DatabusOptions {
    int baudRate, rxPin, txPin, uartNum;
};

struct Mgpu_Databus {
    const Mgpu_Allocator *allocator;
};