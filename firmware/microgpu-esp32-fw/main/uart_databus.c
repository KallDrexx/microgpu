#include <assert.h>
#include <memory.h>
#include "common.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "microgpu-common/common.h"
#include "microgpu-common/databus.h"
#include "microgpu-common/operations/operation_deserializer.h"
#include "microgpu-common/responses/response_serializer.h"
#include "microgpu-common/packet_framing.h"
#include "uart_databus.h"

#define BUFFER_SIZE (1024 * 2)

void log_options(Mgpu_DatabusOptions *options) {
    ESP_LOGI(LOG_TAG, "UART databus options:");
    ESP_LOGI(LOG_TAG, "Baud rate: %u", options->baudRate);
    ESP_LOGI(LOG_TAG, "RX pin: %u", options->rxPin);
    ESP_LOGI(LOG_TAG, "TX pin: %u", options->txPin);
    ESP_LOGI(LOG_TAG, "UART port: %u", options->uartNum);
}

bool parse_receive_buffer(Mgpu_Databus *databus, Mgpu_Operation *operation) {
    mgpu_packet_framing_decode(databus->receiveBuffer, )
}

Mgpu_Databus *mgpu_databus_new(Mgpu_DatabusOptions *options, const Mgpu_Allocator *allocator) {
    assert(options != NULL);
    mgpu_alloc_assert(allocator);

    log_options(options);
    uart_config_t  uartConfig = {
            .baud_rate = options->baudRate,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_param_config(options->uartNum, &uartConfig));
    ESP_ERROR_CHECK(uart_set_pin(options->uartNum, options->txPin, options->rxPin, -1, -1));

    QueueHandle_t uartQueue;
    ESP_ERROR_CHECK(uart_driver_install(options->uartNum, BUFFER_SIZE, BUFFER_SIZE, 10, &uartQueue, 0));

    Mgpu_Databus *databus = allocator->FastMemAllocateFn(sizeof(Mgpu_Databus));
    databus->allocator = allocator;
    databus->uartNum = options->uartNum;
    databus->sendPrepareBuffer = allocator->FastMemAllocateFn(BUFFER_SIZE);
    databus->receiveBuffer = allocator->FastMemAllocateFn(BUFFER_SIZE);

    return databus;
}

void mgpu_databus_free(Mgpu_Databus *databus) {
    if (databus != NULL) {
        databus->allocator->FastMemFreeFn(databus->sendPrepareBuffer);
        databus->allocator->FastMemFreeFn(databus->receiveBuffer);
        databus->allocator->FastMemFreeFn(databus);
    }
}

bool mgpu_databus_get_next_operation(Mgpu_Databus *databus, Mgpu_Operation *operation) {
    assert(databus != NULL);
    assert(operation != NULL);

    memset(operation, 0, sizeof(Mgpu_Operation));
    size_t uartRxLength;
    ESP_ERROR_CHECK(uart_get_buffered_data_len(databus->uartNum, &uartRxLength));
}

void mgpu_databus_send_response(Mgpu_Databus *databus, Mgpu_Response *response) {
    assert(databus != NULL);
    assert(response != NULL);

    int byteCount = mgpu_serialize_response(response, databus->sendPrepareBuffer, BUFFER_SIZE);
    if (byteCount <= 0) {
        switch (byteCount) {
            case MGPU_ERROR_BUFFER_TOO_SMALL:
                ESP_LOGE(LOG_TAG, "Attempted to serialize response, but it required a larger buffer");
                break;

            case MGPU_ERROR_UNKNOWN_RESPONSE_TYPE:
                ESP_LOGE(LOG_TAG, "Attempted to serialize a response, but no serializer exists for that response type");
                break;

            default:
                ESP_LOGE(LOG_TAG, "Response serializing failed with unknown error: %d", byteCount);
                break;
        }

        return;
    }

    ESP_ERROR_CHECK(uart_write_bytes(databus->uartNum, databus->sendPrepareBuffer, byteCount));
}

uint16_t mgpu_databus_get_max_size(Mgpu_Databus *databus) {
    return BUFFER_SIZE;
}

void init_databus_options(Mgpu_DatabusOptions *options) {
    assert(options != NULL);

    options->baudRate = CONFIG_MICROGPU_DATABUS_UART_BAUD_RATE;
    options->txPin = CONFIG_MICROGPU_DATABUS_UART_TX_PIN;
    options->rxPin = CONFIG_MICROGPU_DATABUS_UART_RX_PIN;
    options->uartNum = CONFIG_MICROGPU_DATABUS_UART_PORT_NUM;
}
