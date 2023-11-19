#include <stdlib.h>
#include "messages.h"

char *currentMessage = NULL;

char *mgpu_message_get_pointer(void) {
    if (currentMessage == NULL) {
        // Eventually this needs to use the mgpu_allocator mechanism for creation, but right now
        // I just want to get it out of the stack.
        currentMessage = calloc(MESSAGE_MAX_LEN + 1, sizeof(char));
    }

    return currentMessage;
}
