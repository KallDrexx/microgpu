#include <stdlib.h>
#include "messages.h"

char lastMessage[MESSAGE_MAX_LEN + 1] = {'\0'};
bool messageIsDirty = false;

Mgpu_Message mgpu_message_get_latest(void) {
    return lastMessage;
}

void mgpu_message_set(const char *message) {
    messageIsDirty = true;
    for (int x = 0; x < MESSAGE_MAX_LEN + 1; x++) {
        lastMessage[x] = '\0';
    }

    if (message != NULL) {
        for (int x = 0; x < MESSAGE_MAX_LEN; x++) {
            if (message[x] == '\0') {
                return;
            }

            lastMessage[x] = message[x];
        }
    }
}

Mgpu_Message mgpu_message_get_latest_if_changed(void) {
    if (messageIsDirty) {
        messageIsDirty = false;
        return lastMessage;
    }

    return NULL;
}
