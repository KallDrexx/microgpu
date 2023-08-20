#include <assert.h>
#include "null_databus.h"

Mgpu_Response lastSeenResponse;

size_t mgpu_databus_get_size(Mgpu_DataBusOptions *options) {
    return sizeof(struct Mgpu_Databus);
}

Mgpu_Databus *mgpu_databus_init(void *memory, Mgpu_DataBusOptions *options) {
    return memory;
}

void mgpu_databus_uninit(Mgpu_Databus *databus) {}

bool mgpu_databus_get_next_packet(Mgpu_Databus *databus, Mgpu_Operation *operation) {
    return false;
}

void mgpu_databus_send_response(Mgpu_Databus *databus, Mgpu_Response *response) {
    assert(databus != NULL);
    assert(response != NULL);

    lastSeenResponse.type = response->type;
    switch (response->type) {
        case Mgpu_Response_Status:
            lastSeenResponse.status = response->status;
            break;

        case Mgpu_Response_LastMessage:
            lastSeenResponse.lastMessage = response->lastMessage;
            break;
    }
}

bool mgpu_null_databus_get_last_response(Mgpu_Databus *databus, Mgpu_Response *response) {
    *response = lastSeenResponse;
    return true;
}
