#pragma once

#include <hal/spi_types.h>
#include "microgpu-common/alloc.h"

struct Mgpu_DatabusOptions {
    int copiPin, cipoPin, sclkPin, csPin, handshakePin;
    spi_host_device_t spiHost;
};

struct Mgpu_Databus {
    const Mgpu_Allocator *allocator;
    spi_host_device_t spiHost;
    uint8_t *receiveBuffer, *sendBuffer;
};

void init_databus_options(Mgpu_DatabusOptions *options);
