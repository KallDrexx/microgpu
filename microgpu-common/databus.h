#pragma once

#include <stdint.h>

/*
 * The accessible interface to the databus. The microgpu databus is
 * responsible for receiving operations from a controlling device and
 * sending responses back.
 */
typedef struct Mgpu_Databus Mgpu_Databus;

/*
 * Implementation specific structure that provides options for configuration
 * of the databus.
 */
typedef struct Mgpu_DatabusOptions Mgpu_DataBusOptions;

/*
 * Bytes received by the databus
 */
typedef struct {
    int8_t *bytes;
    size_t size;
} Mgpu_Received_Packet;

/*
 * Gets the size of memory required to be allocated in order to instantiate the databus.
 */
size_t mgpu_databus_get_size(void);

/*
 * Initializes the databus using the passed in pre-allocated memory. The memory area
 * provided must be allocated to the size specified in the return value of
 * `mgpu_databus_get_size()`.
 */
Mgpu_Databus* mgpu_databus_init(void *memory, Mgpu_DataBusOptions *options);

/*
 * Tears down the databus.
 *
 * The caller is responsible for deallocating the memory of the overall databus pointer.
 */
void mgpu_databus_uninit(Mgpu_Databus *databus);

/*
 * Blocks and waits for the databus to receive its next packet. Any previously
 * received packets *must* be discarded before this is called, as the packet's
 * memory may be overwritten by the next packet is received. The packet's bytes
 * will no longer be valid after a call to `mgpu_databus_uninit()`.
 */
Mgpu_Received_Packet mgpu_databus_get_next_packet(Mgpu_Databus *databus);

/*
 * Sends byte data to the controlling device over the databus.
 */
void mgpu_databus_send_packet(Mgpu_Databus *databus, uint8_t* bytes, size_t size);
