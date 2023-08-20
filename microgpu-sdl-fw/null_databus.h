#pragma once

#include "microgpu-common/databus.h"

struct Mgpu_Databus {
    void *nothing;
};

struct Mgpu_DataBusOptions {
    void *nothing;
};

bool mgpu_null_databus_get_last_response(Mgpu_Databus *databus, Mgpu_Response *response);