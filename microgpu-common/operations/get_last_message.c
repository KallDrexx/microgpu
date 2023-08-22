#include "get_last_message.h"
#include "microgpu-common/messages.h"

void mgpu_exec_get_last_message(Mgpu_Databus *databus) {
    Mgpu_Response response = {.type = Mgpu_Response_LastMessage};
    response.lastMessage.message = mgpu_message_get_latest();

    mgpu_databus_send_response(databus, &response);
}
