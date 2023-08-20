#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "operations.h"
#include "responses.h"

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
 * Gets the size of memory required to be allocated in order to instantiate the databus.
 */
size_t mgpu_databus_get_size(Mgpu_DataBusOptions *options);

/*
 * Initializes the databus using the passed in pre-allocated memory. The memory area
 * provided must be allocated to the size specified in the return value of
 * `mgpu_databus_get_size()`.
 */
Mgpu_Databus *mgpu_databus_init(void *memory, Mgpu_DataBusOptions *options);

/*
 * Tears down the databus.
 *
 * The caller is responsible for deallocating the memory of the overall databus pointer.
 */
void mgpu_databus_uninit(Mgpu_Databus *databus);

/*
 * Blocks and waits for the next operation to be received over the databus.
 * Returns true if the passed in operation struct was populated, or false if
 * an error occurred and the struct could not be filled in.
 */
bool mgpu_databus_get_next_packet(Mgpu_Databus *databus, Mgpu_Operation *operation);

/*
 * Sends the specified response to the controller over the databus. Almost always
 * triggered in reaction to an operation that requested a response.
 */
void mgpu_databus_send_response(Mgpu_Databus *databus, Mgpu_Response *response);
