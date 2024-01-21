
#include <string.h>
#include <inttypes.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <driver/spi_common.h>
#include <driver/spi_master.h>
#include <esp_intr_alloc.h>
#include <sx127x.h>
#include "esp_log.h"
#include "esp_err.h"
#include "settings.h"
#include "lora.h"


#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define DIO0 26

static const char* TAG = "LORA";
// #define LOG_LOCAL_LEVEL ESP_LOG_WARN

sx127x *device = NULL;
TaskHandle_t handle_interrupt;

int total_packets_received = 0;

void IRAM_ATTR handle_interrupt_fromisr(void *arg) {
    xTaskResumeFromISR(handle_interrupt);
}

void handle_interrupt_task(void *arg) {
  while (1) {
    vTaskSuspend(NULL);
    sx127x_handle_interrupt((sx127x *)arg);
  }
}

void rx_callback(sx127x *device, uint8_t *data, uint16_t data_length) {
    uint8_t payload[514];
    for (size_t i = 0; i < data_length; i++) {
        payload[i] = data[i];
    }
    payload[data_length] = '\0';

    int16_t rssi;
    ESP_ERROR_CHECK(sx127x_rx_get_packet_rssi(device, &rssi));
    float snr;
    ESP_ERROR_CHECK(sx127x_lora_rx_get_packet_snr(device, &snr));
    int32_t frequency_error;
    ESP_ERROR_CHECK(sx127x_rx_get_frequency_error(device, &frequency_error));
    ESP_LOGI(TAG, "received: %d %s rssi: %d snr: %f", data_length, payload, rssi, snr);
    printf("message %s\n", payload);
    total_packets_received++;
}

void cad_callback(sx127x *device, int cad_detected) {
    if (cad_detected == 0) {
        ESP_LOGI(TAG, "cad not detected");
        ESP_ERROR_CHECK(sx127x_set_opmod(SX127x_MODE_CAD, SX127x_MODULATION_LORA, device));
        return;
    }
    // put into RX mode first to handle interrupt as soon as possible
    ESP_ERROR_CHECK(sx127x_set_opmod(SX127x_MODE_RX_CONT, SX127x_MODULATION_LORA, device));
    ESP_LOGI(TAG, "cad detected\n");
}

void setup_gpio_interrupts(gpio_num_t gpio, sx127x *device) {
    gpio_set_direction(gpio, GPIO_MODE_INPUT);
    gpio_pulldown_en(gpio);
    gpio_pullup_dis(gpio);
    gpio_set_intr_type(gpio, GPIO_INTR_POSEDGE);
    gpio_isr_handler_add(gpio, handle_interrupt_fromisr, (void *)device);
}

esp_err_t initialize_lora() {
    ESP_LOGI(TAG, "Read LoRa settings");
    settings_handle_t settings_handle;
    settings_create(&settings_handle);

    // Validations

    ESP_LOGI(TAG, "starting up lora");
    spi_bus_config_t config = {
        .mosi_io_num = MOSI,
        .miso_io_num = MISO,
        .sclk_io_num = SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &config, 1));
    spi_device_interface_config_t dev_cfg = {
        .clock_speed_hz = 3E6,
        .spics_io_num = SS,
        .queue_size = 16,
        .command_bits = 0,
        .address_bits = 8,
        .dummy_bits = 0,
        .mode = 0};
    spi_device_handle_t spi_device;
    ESP_ERROR_CHECK(spi_bus_add_device(HSPI_HOST, &dev_cfg, &spi_device));
    ESP_ERROR_CHECK(sx127x_create(spi_device, &device));
    ESP_ERROR_CHECK(sx127x_set_opmod(SX127x_MODE_SLEEP, SX127x_MODULATION_LORA, device));
    ESP_ERROR_CHECK(sx127x_set_frequency(437200012, device));
    ESP_ERROR_CHECK(sx127x_lora_reset_fifo(device));
    ESP_ERROR_CHECK(sx127x_rx_set_lna_boost_hf(true, device));
    ESP_ERROR_CHECK(sx127x_set_opmod(SX127x_MODE_STANDBY, SX127x_MODULATION_LORA, device));
    ESP_ERROR_CHECK(sx127x_rx_set_lna_gain(SX127x_LNA_GAIN_G4, device));
    ESP_ERROR_CHECK(sx127x_lora_set_bandwidth(SX127x_BW_125000, device));
    ESP_ERROR_CHECK(sx127x_lora_set_implicit_header(NULL, device));
    ESP_ERROR_CHECK(sx127x_lora_set_modem_config_2(SX127x_SF_9, device));
    ESP_ERROR_CHECK(sx127x_lora_set_syncword(18, device));
    ESP_ERROR_CHECK(sx127x_set_preamble_length(8, device));
    sx127x_rx_set_callback(rx_callback, device);
    sx127x_lora_cad_set_callback(cad_callback, device);

    BaseType_t task_code = xTaskCreatePinnedToCore(handle_interrupt_task, "handle interrupt", 8196, device, 2, &handle_interrupt, xPortGetCoreID());
    if (task_code != pdPASS) {
        ESP_LOGE(TAG, "can't create task %d", task_code);
        sx127x_destroy(device);
        return ESP_OK;
    } else {
        ESP_LOGI(TAG, "LoRa task created %d", task_code);
    }

    gpio_install_isr_service(0);
    setup_gpio_interrupts((gpio_num_t)DIO0, device);

    ESP_ERROR_CHECK(sx127x_set_opmod(SX127x_MODE_RX_CONT, SX127x_MODULATION_LORA, device));
    return ESP_OK;
}