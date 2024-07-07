#include <assert.h>
#include "common.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "microgpu-common/common.h"
#include "microgpu-common/databus.h"
#include "uart_databus.h"

#define BUFFER_SIZE (1024 * 2)

void log_options(Mgpu_DatabusOptions *options) {
    ESP_LOGI(LOG_TAG, "UART databus options:");
    ESP_LOGI(LOG_TAG, "Baud rate: %u", options->baudRate);
    ESP_LOGI(LOG_TAG, "RX pin: %u", options->rxPin);
    ESP_LOGI(LOG_TAG, "TX pin: %u", options->txPin);
    ESP_LOGI(LOG_TAG, "UART port: %u", options->uartNum);
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
}

void mgpu_databus_free(Mgpu_Databus *databus) {

}

bool mgpu_databus_get_next_operation(Mgpu_Databus *databus, Mgpu_Operation *operation) {

}

void mgpu_databus_send_response(Mgpu_Databus *databus, Mgpu_Response *response) {

}

uint16_t mgpu_databus_get_max_size(Mgpu_Databus *databus) {

}
