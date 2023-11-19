#pragma once

#include "microgpu-common/databus.h"

struct Mgpu_Databus {
    const Mgpu_Allocator *allocator;
};

struct Mgpu_DatabusOptions {
    void *nothing;
};


void init_databus_options(Mgpu_DatabusOptions *options);
