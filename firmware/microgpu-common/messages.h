#pragma once

#include <stdbool.h>

#define MESSAGE_MAX_LEN 1023

/*
 * Gets the pointer to the message buffer.
 *
 * The pointer points to a static and re-used lastMessage buffer. If
 * the consumer needs the message to last longer the consumer must copy it into
 * its own buffer.
 */
char *mgpu_message_get_pointer(void);

/*
 * Gets the pointer to the message only if it has changed since the last time this
 * function was called. If the message is the same then it returns NULL.
 */
char *mgpu_message_get_latest_if_changed(void);
