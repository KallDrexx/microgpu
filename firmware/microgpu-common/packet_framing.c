#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include "packet_framing.h"
#include "messages.h"

int mgpu_packet_framing_encode(const uint8_t *msg_buffer,
                               size_t msg_size,
                               uint8_t *target_buffer,
                               size_t buffer_size) {
    assert(msg_buffer != NULL);
    assert(target_buffer != NULL);

    if (msg_size == 0) {
        return 0; // Nothing to write
    }

    if (msg_size > MGPU_FRAMING_MAX_MSG_SIZE) {
        return MGPU_FRAMING_ERROR_MSG_TOO_LARGE;
    }

    // We are taking the message in the bytes, a two byte checksum value, and then
    // encoding the message with Consistent Overhead Stuffing. This means each
    // packet has 4 extra bytes added to it. So lets make sure the target buffer can hold that
    size_t final_size = msg_size + 4;
    if (final_size > buffer_size) {
        return MGPU_FRAMING_ERROR_BUFFER_TOO_SMALL;
    }

    // Encode the message with Consistent Overhead Byte Stuffing encoding, but with
    // checksum as part of the message. This entails changing all
    // zero byte values to be a value of how many bytes to the next zero byte value.
    uint16_t checksum = 0;

    // First byte is the COBS zero byte offset value
    memcpy(target_buffer + 1, msg_buffer, msg_size);
    target_buffer[final_size - 1] = 0;

    uint8_t index_of_last_zero_byte = 0;
    uint8_t offset;
    for (size_t x = 0; x < msg_size; x++) {
        size_t index = x + 1; // Message starts at index 1 for initial zero offset byte
        checksum += target_buffer[index];

        if (target_buffer[index] != 0) {
            continue; // leave non-zero bytes alone
        }

        offset = index - index_of_last_zero_byte;
        target_buffer[index_of_last_zero_byte] = offset;
        index_of_last_zero_byte = index;
    }

    // Add the checksum to the target buffer
    uint8_t checksumByte0 = (uint8_t)(checksum >> 8);
    if (checksumByte0 != 0) {
        target_buffer[final_size - 3] = checksumByte0;
    } else {
        offset = msg_size + 1 - index_of_last_zero_byte;
        target_buffer[index_of_last_zero_byte] = offset;
        index_of_last_zero_byte = final_size - 3;
    }

    uint8_t checksumByte1 = (uint8_t)(checksum);
    if (checksumByte1 != 0) {
        target_buffer[final_size - 2] = checksumByte1;
    } else {
        offset = msg_size + 2 - index_of_last_zero_byte;
        target_buffer[index_of_last_zero_byte] = offset;
        index_of_last_zero_byte = final_size - 2;
    }

    // Add the final zero byte offset
    offset = final_size - 1 - index_of_last_zero_byte;
    target_buffer[index_of_last_zero_byte] = offset;

    return (int) final_size;
}

void mgpu_packet_framing_decode(const uint8_t *input_buffer,
                                size_t input_buffer_size,
                                uint8_t *decode_buffer,
                                size_t decode_buffer_size,
                                size_t *decoded_byte_count,
                                size_t *input_bytes_processed) {
    assert(input_buffer != NULL);
    assert(decode_buffer != NULL);
    assert(decoded_byte_count != NULL);
    assert(input_bytes_processed != NULL);

    *decoded_byte_count = 0;
    int index_of_zero = -1;
    for (int index = 0; index < input_buffer_size; index++) {
        if (input_buffer[index] == 0) {
            index_of_zero = index;
            break;
        }
    }

    *input_bytes_processed = index_of_zero + 1;
    if (index_of_zero <= 0) {
        return;
    }

    if (*input_bytes_processed < 5) {
        // Minimum of 5 bytes (1 offset + 2 checksum + 1 delimiter + 1 data byte)
        char *message = mgpu_message_get_pointer();
        assert(message != NULL);
        snprintf(message, MESSAGE_MAX_LEN, "Received %zu sized packet, which is too few", *input_bytes_processed);
        return;
    }

    // Move everything up to the zero delimiter and the initial offset byte to the decode buffer
    size_t decode_size_required = index_of_zero - 1;
    size_t packet_size = decode_size_required - 2;
    if (decode_buffer_size < decode_size_required) {
        // Not enough bytes in input buffer.
        char *message = mgpu_message_get_pointer();
        assert(message != NULL);
        snprintf(message,
                 MESSAGE_MAX_LEN,
                 "Received packet of size %zu, but decode buffer only has %zu capacity",
                 decode_size_required,
                 decode_buffer_size);

        return;
    }

    memcpy(decode_buffer, input_buffer + 1, decode_size_required); // skip the initial offset byte

    // iterate through the bytes, correcting any zeros and calculating the checksum
    uint16_t calculatedChecksum = 0;
    uint8_t offsetToNextZero = input_buffer[0];
    for (int x = 0; x < packet_size; x++) {
        offsetToNextZero--;
        if (offsetToNextZero == 0) {
            offsetToNextZero = decode_buffer[x];
            decode_buffer[x] = 0;
        } else {
            calculatedChecksum += decode_buffer[x];
        }
    }

    // Get the expected checksum, and zero out the bytes if required
    uint8_t checksumBytes[] = {
            decode_buffer[decode_size_required - 2],
            decode_buffer[decode_size_required - 1],
    };

    for (int x = 0; x < sizeof(checksumBytes); x++) {
        offsetToNextZero--;
        if (offsetToNextZero == 0) {
            offsetToNextZero = checksumBytes[x];
            checksumBytes[x] = 0;
        }
    }

    if (offsetToNextZero != 1) {
        char *message = mgpu_message_get_pointer();
        assert(message != NULL);
        snprintf(message,
                 MESSAGE_MAX_LEN,
                 "Expected zero offset to match the end of the packet, instead had an offset of %u",
                 offsetToNextZero);

        return;
    }

    uint16_t expectedChecksum = (((uint16_t) checksumBytes[0]) << 8) | ((uint16_t) checksumBytes[1]);
    if (calculatedChecksum != expectedChecksum) {
        char *message = mgpu_message_get_pointer();
        assert(message != NULL);
        snprintf(message, MESSAGE_MAX_LEN, "Received packet with incorrect checksum value");

        return;
    }

    // Decode was successful
    *decoded_byte_count = packet_size;
}
