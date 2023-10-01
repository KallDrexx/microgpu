#include <driver/spi_slave.h>
#include <driver/gpio.h>
#include <memory.h>
#include "esp_log.h"
#include "microgpu-common/common.h"
#include "microgpu-common/databus.h"
#include "microgpu-common/operation_deserializer.h"
#include "microgpu-common/response_serializer.h"
#include "spi_databus.h"
#include "common.h"

#define BUFFER_SIZE 1024
WORD_ALIGNED_ATTR uint8_t receiveBuffer[BUFFER_SIZE];
WORD_ALIGNED_ATTR uint8_t sendBuffer[BUFFER_SIZE];

int handshakePin;

void log_options(Mgpu_DatabusOptions *options) {
    ESP_LOGI(LOG_TAG, "SPI databus options:");
    ESP_LOGI(LOG_TAG, "COPI pin: %u", options->copiPin);
    ESP_LOGI(LOG_TAG, "CIPO pin: %u", options->cipoPin);
    ESP_LOGI(LOG_TAG, "SCLK pin: %u", options->sclkPin);
    ESP_LOGI(LOG_TAG, "Chip Select pin: %u", options->csPin);
    ESP_LOGI(LOG_TAG, "Handshake pin: %u", options->handshakePin);
    ESP_LOGI(LOG_TAG, "SPI host: %u", options->spiHost);
}

void spi_ready_callback(spi_slave_transaction_t *transaction) {
    gpio_set_level(handshakePin, 1);
}

void spi_unready_callback(spi_slave_transaction_t *transaction) {
    gpio_set_level(handshakePin, 0);
}

Mgpu_Databus *mgpu_databus_new(Mgpu_DatabusOptions *options, const Mgpu_Allocator *allocator) {
    assert(options != NULL);
    assert(allocator != NULL);

    log_options(options);
    handshakePin = options->handshakePin;

    spi_bus_config_t busConfig = {
            .mosi_io_num = options->copiPin,
            .miso_io_num = options->cipoPin,
            .sclk_io_num = options->sclkPin,
            .quadhd_io_num = -1,
            .quadwp_io_num = -1,
    };

    spi_slave_interface_config_t slaveConfig = {
            .mode = 0,
            .spics_io_num = options->csPin,
            .queue_size = 3,
            .flags = 0,
            .post_setup_cb = spi_ready_callback,
            .post_trans_cb = spi_unready_callback,
    };

    gpio_config_t handshakeConfig = {
            .intr_type = GPIO_INTR_DISABLE,
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = (1 << options->handshakePin),
    };

    gpio_config(&handshakeConfig);

    //Enable pull-ups on SPI lines, so we don't detect rogue pulses when no master is connected.
    gpio_set_pull_mode(options->copiPin, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(options->sclkPin, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(options->csPin, GPIO_PULLUP_ONLY);

    ESP_ERROR_CHECK(spi_slave_initialize(options->spiHost, &busConfig, &slaveConfig, SPI_DMA_CH_AUTO));

    Mgpu_Databus *databus = allocator->AllocateFn(sizeof(Mgpu_Databus));
    databus->allocator = allocator;
    databus->spiHost = options->spiHost;

    return databus;
}

void mgpu_databus_free(Mgpu_Databus *databus) {
    if (databus) {
        databus->allocator->FreeFn(databus);
    }
}

bool mgpu_databus_get_next_operation(Mgpu_Databus *databus, Mgpu_Operation *operation) {
    assert(databus != NULL);
    assert(operation != NULL);

    spi_slave_transaction_t transaction;
    memset(&transaction, 0, sizeof(transaction));
    memset(receiveBuffer, 0, BUFFER_SIZE);
    memset(sendBuffer, 0, BUFFER_SIZE);

    // Set up the SPI transaction
    transaction.length = BUFFER_SIZE * 8;
    transaction.rx_buffer = receiveBuffer;
    transaction.tx_buffer = sendBuffer;

    esp_err_t result = spi_slave_transmit(databus->spiHost, &transaction, portMAX_DELAY);
    if (result != ESP_OK) {
        ESP_LOGE(LOG_TAG, "SPI receive failed: %s", esp_err_to_name(result));
        return false;
    }

    size_t length = min(transaction.length, transaction.trans_len);

    if (!mgpu_operation_deserialize(receiveBuffer, length, operation)) {
        ESP_LOGW(LOG_TAG, "Failed to deserialize SPI transaction");

        printf("data: ");
        for (int x = 0; x < length; x++) {
            printf("%02X ", receiveBuffer[x]);
        }

        printf("\n");
        return false;
    }

    return true;
}

void mgpu_databus_send_response(Mgpu_Databus *databus, Mgpu_Response *response) {
    assert(databus != NULL);
    assert(response != NULL);

    memset(sendBuffer, 0, BUFFER_SIZE);
    int byteCount = mgpu_serialize_response(response, sendBuffer + 2, BUFFER_SIZE - 2);
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

    sendBuffer[0] = (uint8_t)(byteCount >> 8);
    sendBuffer[1] = (uint8_t)(byteCount & 0xFF);

    spi_slave_transaction_t transaction;
    memset(&transaction, 0, sizeof(transaction));
    memset(receiveBuffer, 0, BUFFER_SIZE);

    // Set up the SPI transaction
    transaction.length = BUFFER_SIZE * 8;
    transaction.rx_buffer = receiveBuffer;
    transaction.tx_buffer = sendBuffer;

    esp_err_t spiResult = spi_slave_transmit(databus->spiHost, &transaction, portMAX_DELAY);
    if (spiResult != ESP_OK) {
        ESP_LOGE(LOG_TAG, "SPI send failed: %s", esp_err_to_name(spiResult));
    }
}
