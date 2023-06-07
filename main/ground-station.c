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
#define EXAMPLE_HTTP_REQUEST_SIZE 16384
#define OTA_WAIT_PERIOD_MS 1000 * 30 // (1000 * 60 * 1)

static const char* TAG = "GroundStation";
extern const uint8_t server_cert_pem_start[] asm("_binary_amazonaws_com_root_cert_pem_start");
extern const uint8_t server_cert_pem_end[] asm("_binary_amazonaws_com_root_cert_pem_end");

static esp_err_t validate_image_header(esp_app_desc_t *new_app_info)
{
    if (new_app_info == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_app_desc_t running_app_info;
    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
        ESP_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
    }

    if (memcmp(new_app_info->version, running_app_info.version, sizeof(new_app_info->version)) == 0) {
        ESP_LOGW(TAG, "Current running version is the same as a new. We will not continue the update.");
        return ESP_ERR_INVALID_VERSION;
    }

    return ESP_OK;
}

void ota_task(void *pvParameter) {
    ESP_LOGI(TAG, "Starting OTA Task");
    esp_err_t ota_finish_err = ESP_OK;

    esp_http_client_config_t http_config = {
      .url = EXAMPLE_FIRMWARE_UPGRADE_URL,
      .cert_pem = (char *)server_cert_pem_start,
      .keep_alive_enable = true,
    };
    esp_https_ota_config_t ota_config = {
      .http_config = &http_config,
      .partial_http_download = true,
      .max_http_request_size = EXAMPLE_HTTP_REQUEST_SIZE,
    };

    while (true) {
      ESP_LOGI(TAG, "Search for OTA updates...");
      esp_https_ota_handle_t https_ota_handle = NULL;
      esp_err_t err = esp_https_ota_begin(&ota_config, &https_ota_handle);
      if (err != ESP_OK) {
        ESP_LOGE(TAG, "ESP HTTPS OTA Begin failed");
        goto abort_update;
      }

      esp_app_desc_t app_desc;
      err = esp_https_ota_get_img_desc(https_ota_handle, &app_desc);
      if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_https_ota_read_img_desc failed");
        goto abort_update;
      }

      err = validate_image_header(&app_desc);
      if (err != ESP_OK) {
        if (err != ESP_ERR_INVALID_VERSION) {
          ESP_LOGE(TAG, "image header verification failed");
          goto abort_update;
        }
        goto task_end;
      }

      while(true) {
          err = esp_https_ota_perform(https_ota_handle);
          if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
            break;
          }

          ESP_LOGI(TAG, "Image bytes read: %d", esp_https_ota_get_image_len_read(https_ota_handle));
      }

      if (esp_https_ota_is_complete_data_received(https_ota_handle) != true) {
        // the OTA image was not completely received and user can customise the response to this situation.
        ESP_LOGE(TAG, "Complete data was not received.");
      } else {
        ESP_LOGI(TAG, "Complete data was received.");
        ota_finish_err = esp_https_ota_finish(https_ota_handle);
        if ((err == ESP_OK) && (ota_finish_err == ESP_OK)) {
          ESP_LOGI(TAG, "ESP_HTTPS_OTA upgrade successful. Rebooting ...");
          vTaskDelay(1000 / portTICK_PERIOD_MS);
          esp_restart();
        } else {
          if (ota_finish_err == ESP_ERR_OTA_VALIDATE_FAILED) {
            ESP_LOGE(TAG, "Image validation failed, image is corrupted");
          }
          ESP_LOGE(TAG, "ESP_HTTPS_OTA upgrade failed 0x%x", ota_finish_err);
        }
      }
abort_update:
      esp_https_ota_abort(https_ota_handle);
      ESP_LOGE(TAG, "ESP_HTTPS_OTA upgrade failed");
task_end:
      vTaskDelay(OTA_WAIT_PERIOD_MS / portTICK_PERIOD_MS);
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
