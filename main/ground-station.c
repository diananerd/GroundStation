#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_console.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "cmd_wifi.h"
#include "api_calls.h"
#include "cmd_api.h"

#define EXAMPLE_FIRMWARE_UPGRADE_URL "https://platzi-ground-station-beta.s3.us-east-2.amazonaws.com/firmware/1.0.6/ground-station.bin"

static const char* TAG = "GroundStation";
extern const uint8_t server_cert_pem_start[] asm("_binary_amazonaws_com_root_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_amazonaws_com_root_cert_pem_end");

void ota_task(void *pvParameter) {
    ESP_LOGI(TAG, "Starting OTA Task");
    esp_http_client_config_t http_config = {
      .url = EXAMPLE_FIRMWARE_UPGRADE_URL,
      .cert_pem = (char *)server_cert_pem_start,
    };
    esp_https_ota_config_t config = {
      .http_config = &http_config,
    };
    for (;;) {
      ESP_LOGI(TAG, "Search for OTA updates...");
      esp_err_t ret = esp_https_ota(&config);
      if (ret == ESP_OK) {
        ESP_LOGI(TAG, "Upgrade firmware!");
        esp_restart();
      } else {
        ESP_LOGI(TAG, "The firmware version is up-to-date");
      }
      vTaskDelay((1000 * 60 * 1) / portTICK_PERIOD_MS);
    }
}

static void initialize_nvs(void) {
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
}

void app_main(void) {
  /* Log git version build */
  const esp_app_desc_t* app_description;
  app_description = esp_app_get_description();

  ESP_LOGI(TAG, "Build version %s", app_description->version);

  /* Prepare serial console for REPL */
  esp_console_repl_t *repl = NULL;
  esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
  repl_config.prompt = ">";

  initialize_nvs();
  initialize_wifi();
  initialize_api();

  /* Register commands */
  esp_console_register_help_command();
  register_wifi();
  register_api();

  /* Setup console REPL over UART */
  esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));

  /* Run REPL */
  ESP_ERROR_CHECK(esp_console_start_repl(repl));

  xTaskCreate(&ota_task, "ota_task", 1024 * 8, NULL, 5, NULL);
}
