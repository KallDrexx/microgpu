#include <stdlib.h>
#include "messages.h"

char lastMessage[MESSAGE_MAX_LEN + 1] = {'\0'};
bool messageIsDirty = false;

char *mgpu_message_get_pointer(void) {
    return lastMessage;
}

char *mgpu_message_get_latest_if_changed(void) {
    if (messageIsDirty) {
        messageIsDirty = false;
        return lastMessage;
    }

    return NULL;
}
