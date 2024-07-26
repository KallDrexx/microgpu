#include <driver/spi_slave.h>
#include <driver/gpio.h>
#include <memory.h>
#include "esp_log.h"
#include "microgpu-common/common.h"
#include "microgpu-common/databus.h"
#include "microgpu-common/operations/operation_deserializer.h"
#include "microgpu-common/packet_framing.h"
#include "microgpu-common/responses/response_serializer.h"
#include "spi_databus.h"
#include "common.h"

#define SPI_BUFFER_SIZE (1024)
#define PACKET_BUFFER_SIZE (255)

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
    mgpu_alloc_assert(allocator);

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
            .pull_down_en = GPIO_PULLDOWN_ENABLE,
    };

    gpio_config(&handshakeConfig);

    //Enable pull-ups on SPI lines, so we don't detect rogue pulses when no master is connected.
    gpio_set_pull_mode(options->copiPin, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(options->sclkPin, GPIO_PULLUP_ONLY);
    gpio_set_pull_mode(options->csPin, GPIO_PULLUP_ONLY);

    ESP_ERROR_CHECK(spi_slave_initialize(options->spiHost, &busConfig, &slaveConfig, SPI_DMA_CH_AUTO));

    Mgpu_Databus *databus = allocator->FastMemAllocateFn(sizeof(Mgpu_Databus));
    databus->allocator = allocator;
    databus->spiHost = options->spiHost;
    databus->receiveBuffer = heap_caps_malloc(SPI_BUFFER_SIZE, MALLOC_CAP_DMA);
    databus->sendBuffer = heap_caps_malloc(SPI_BUFFER_SIZE, MALLOC_CAP_DMA);
    databus->encodeDecodeBuffer = allocator->FastMemAllocateFn(PACKET_BUFFER_SIZE);
    databus->receiveBufferBytesRemaining = 0;

    return databus;
}

void mgpu_databus_free(Mgpu_Databus *databus) {
    if (databus) {
        databus->allocator->FastMemFreeFn(databus->encodeDecodeBuffer);
        free(databus->receiveBuffer);
        free(databus->sendBuffer);
        databus->allocator->FastMemFreeFn(databus);
    }
}

bool mgpu_databus_get_next_operation(Mgpu_Databus *databus, Mgpu_Operation *operation) {
    assert(databus != NULL);
    assert(operation != NULL);

    spi_slave_transaction_t transaction;
    memset(&transaction, 0, sizeof(transaction));
    if (databus->receiveBufferBytesRemaining == 0) {
        memset(databus->receiveBuffer, 0, SPI_BUFFER_SIZE);
        memset(databus->sendBuffer, 0, SPI_BUFFER_SIZE);

        // Set up the SPI transaction
        transaction.length = SPI_BUFFER_SIZE * 8;
        transaction.rx_buffer = databus->receiveBuffer;
        transaction.tx_buffer = databus->sendBuffer;

        esp_err_t result = spi_slave_transmit(databus->spiHost, &transaction, portMAX_DELAY);
        if (result != ESP_OK) {
            ESP_LOGE(LOG_TAG, "SPI receive failed: %s", esp_err_to_name(result));
            return false;
        }

        size_t length = min(transaction.length / 8, transaction.trans_len / 8);
        assert(length <= SPI_BUFFER_SIZE);
        databus->receiveBufferBytesRemaining = length;
    }

    size_t inputBytesProcessed = 0, decodedBytes = 0;
    mgpu_packet_framing_decode(databus->receiveBuffer,
                               databus->receiveBufferBytesRemaining,
                               databus->encodeDecodeBuffer,
                               PACKET_BUFFER_SIZE,
                               &decodedBytes,
                               &inputBytesProcessed);

    if (inputBytesProcessed == 0) {
        // We didn't get a complete packet, so discard the rest of the buffer
        databus->receiveBufferBytesRemaining = 0;
        return false;
    }

    if (decodedBytes == 0) {
        // We had bytes but did not have a valid packet
        if (inputBytesProcessed > 1) { // ignore empty packets (usually trailing zeros)
            ESP_LOGW(LOG_TAG, "Failed to deserialize packet from SPI transaction");
            printf("data: ");
            for (int x = 0; x < inputBytesProcessed; x++) {
                printf("%02X ", databus->receiveBuffer[x]);
            }

            printf("\n");
        }
    }

    size_t bytesToShift = databus->receiveBufferBytesRemaining - inputBytesProcessed;
    databus->receiveBufferBytesRemaining = bytesToShift;
    if (bytesToShift > 0) {
        memmove(databus->receiveBuffer, databus->receiveBuffer + inputBytesProcessed, bytesToShift);
    }

    return mgpu_operation_deserialize(databus->encodeDecodeBuffer, decodedBytes, operation);
}

void mgpu_databus_send_response(Mgpu_Databus *databus, Mgpu_Response *response) {
    assert(databus != NULL);
    assert(response != NULL);

    memset(databus->sendBuffer, 0, SPI_BUFFER_SIZE);
    int byteCount = mgpu_serialize_response(response, databus->encodeDecodeBuffer, PACKET_BUFFER_SIZE);
    if (byteCount <= 0) {
        switch (byteCount) {
            case MGPU_ERROR_BUFFER_TOO_SMALL:
                ESP_LOGE(LOG_TAG, "Attempted to serialize response, but it required a larger buffer");
                return;

            case MGPU_ERROR_UNKNOWN_RESPONSE_TYPE:
                ESP_LOGE(LOG_TAG, "Attempted to serialize a response, but no serializer exists for that response type");
                return;

            default:
                ESP_LOGE(LOG_TAG, "Response serializing failed with unknown error: %d", byteCount);
                return;
        }
    }

    int bytesEncoded = mgpu_packet_framing_encode(databus->encodeDecodeBuffer, byteCount, databus->sendBuffer, SPI_BUFFER_SIZE);
    if (bytesEncoded <= 0) {
        switch (bytesEncoded) {
            case MGPU_FRAMING_ERROR_BUFFER_TOO_SMALL:
                ESP_LOGE(LOG_TAG, "The SPI buffer was too small to hold the response");
                return;

            case MGPU_FRAMING_ERROR_MSG_TOO_LARGE:
                ESP_LOGE(LOG_TAG, "The response of type %u was too large to be encoded (%u bytes)", response->type, byteCount);
                return;

            default:
                ESP_LOGE(LOG_TAG, "Encoding response of type %u failed with error: %d", response->type, bytesEncoded);
                return;
        }
    }

    spi_slave_transaction_t transaction;
    memset(&transaction, 0, sizeof(transaction));
    memset(databus->receiveBuffer, 0, SPI_BUFFER_SIZE);

    // Set up the SPI transaction
    transaction.length = SPI_BUFFER_SIZE * 8;
    transaction.rx_buffer = databus->receiveBuffer;
    transaction.tx_buffer = databus->sendBuffer;

    esp_err_t spiResult = spi_slave_transmit(databus->spiHost, &transaction, portMAX_DELAY);
    if (spiResult != ESP_OK) {
        ESP_LOGE(LOG_TAG, "SPI send failed: %s", esp_err_to_name(spiResult));
    }
}

uint16_t mgpu_databus_get_max_size(Mgpu_Databus *databus) {
    assert(databus != NULL);

    return SPI_BUFFER_SIZE;
}

void init_databus_options(Mgpu_DatabusOptions *options) {
    assert(options != NULL);
    ESP_LOGI(LOG_TAG, "Initializing SPI databus");
    options->copiPin = CONFIG_MICROGPU_DATABUS_SPI_COPI;
    options->cipoPin = CONFIG_MICROGPU_DATABUS_SPI_CIPO;
    options->sclkPin = CONFIG_MICROGPU_DATABUS_SPI_SCK;
    options->csPin = CONFIG_MICROGPU_DATABUS_SPI_CS;
    options->handshakePin = CONFIG_MICROGPU_DATABUS_SPI_HANDSHAKE;
    options->spiHost = SPI2_HOST;
}
