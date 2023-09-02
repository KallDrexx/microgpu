#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "operations.h"

/*
 * Takes a series of bytes and attempts to deserialize an operation it. Returns
 * true if the deserialization was successful, or false if deserialization fails.
 */
bool mgpu_operation_deserialize(const uint8_t bytes[], size_t size, Mgpu_Operation *operation);
