#pragma once

// Handles logic for encoding bytes into framed packets, and unpacking framed packets
#include <stddef.h>
#include <stdint.h>

#define MGPU_FRAMING_MAX_MSG_SIZE 250
#define MGPU_FRAMING_ERROR_MSG_TOO_LARGE (-1)
#define MGPU_FRAMING_ERROR_BUFFER_TOO_SMALL (-2)

/* Takes the message buffer container containing the bytes of the message to
 * encode into a packet. Each message should not be larger than the max packet
 * size (250 bytes).
 *
 * Returns the number of bytes written to the target buffer, or a negative
 * value if an error occurs.
 */
int mgpu_packet_framing_encode(const uint8_t *msg_buffer,
                                size_t msg_size,
                                uint8_t *target_buffer,
                                size_t buffer_size);

/* Takes a buffer containing bytes and attempts to decode the first
 * packet it finds ending with a zero byte value. If the packet is a
 * valid MGPU packet, it's placed in the decode buffer with the number
 * of bytes in the decoded packet in `decoded_byte_count`.
 */
void mgpu_packet_framing_decode(const uint8_t *input_buffer,
                                size_t input_buffer_size,
                                uint8_t *decode_buffer,
                                size_t decode_buffer_size,
                                size_t *decoded_byte_count,
                                size_t *input_bytes_processed);
