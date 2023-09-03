#pragma once

#include "responses.h"

#define MGPU_ERROR_BUFFER_TOO_SMALL (-1)
#define MGPU_ERROR_UNKNOWN_RESPONSE_TYPE (-2)

/*
 * Serializes a response into bytes. Returns the number of bytes written into the buffer,
 * or a negative value if an error occurs.
 */
int mgpu_serialize_response(Mgpu_Response *response, uint8_t buffer[], size_t bufferSize);
