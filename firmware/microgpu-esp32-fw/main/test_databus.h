#pragma once

#include "microgpu-common/databus.h"

struct Mgpu_Databus {
    const Mgpu_Allocator *allocator;
};

struct Mgpu_DatabusOptions {
    void *nothing;
};

bool mgpu_test_databus_get_last_response(Mgpu_Databus *databus, Mgpu_Response *response);

void mgpu_test_databus_trigger_reset(Mgpu_Databus *databus);