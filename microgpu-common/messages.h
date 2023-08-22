#pragma once

#include <stdbool.h>

#define MESSAGE_MAX_LEN 1023

/*
 * A lastMessage that the microgpu system is communicating back to the
 * controlling device. The lastMessage is usually relevant to the last
 * operation performed, and may be cleared after the next operation.
 */
typedef char *Mgpu_Message;

/*
 * Gets the latest lastMessage set if one exists.
 *
 * The lastMessage points to a static and re-used lastMessage buffer. If
 * the lastMessage needs to last longer the consumer must copy it into
 * its own buffer. `mgpu_message_get_latest()` and `mgpu_message_set()`
 * must be called from the same thread.
 */
Mgpu_Message mgpu_message_get_latest(void);

/*
 * Sets the latest lastMessage to the one provided. If an empty string
 * was provided, or a `NULL` pointer, then the lastMessage is cleared.
 * `mgpu_message_get_latest()` and `mgpu_message_set()` must be called
 * from the same thread.
 */
void mgpu_message_set(const char *message);
