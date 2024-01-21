#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_console.h"
#include "esp_ota_ops.h"
#include "nvs_flash.h"
#include "settings.h"
#include "settings_console.h"
#include "wifi.h"
#include "wifi_console.h"
#include "oled.h"
#include "oled_console.h"
#include "ota.h"
#include "ota_console.h"
#include "api.h"
#include "api_console.h"
#include "lora.h"
#include "lora_console.h"
#include "motors.h"
#include "motors_console.h"

static const char* TAG = "GROUND_STATION";

extern const char ota_cert_pem_start[] asm("_binary_amazonaws_com_root_cert_pem_start");
extern const char ota_cert_pem_end[] asm("_binary_amazonaws_com_root_cert_pem_end");

extern const char api_cert_pem_start[] asm("_binary_platzi_com_root_cert_pem_start");
extern const char api_cert_pem_end[] asm("_binary_platzi_com_root_cert_pem_end");

static void log_env_variables() {
  # ifndef CONFIG_FIRMWARE_URL
  ESP_LOGE(TAG, "FIRMWARE_URL is not defined in project build");
  # else
  ESP_LOGI(TAG, "FIRMWARE_URL=%s", CONFIG_FIRMWARE_URL);
  # endif
  # ifndef CONFIG_API_BASE_URL
  ESP_LOGE(TAG, "API_BASE_URL is not defined in project build");
  # else
  ESP_LOGI(TAG, "API_BASE_URL=%s", CONFIG_API_BASE_URL);
  #endif
}

void app_main(void) {
  /* Log git version build */
  const esp_app_desc_t* app_description;
  app_description = esp_app_get_description();

  ESP_LOGI(TAG, "Build version %s", app_description->version);

  log_env_variables();

  // Initialize NVS
  ESP_LOGI(TAG, "Initialize nvs flash");
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK( err );

  settings_handle_t settings_handle;
  settings_create(&settings_handle);

  setting_t bs = {};

  // BOARD MODEL
  bs.key = "board_name";
  err = settings_get(&settings_handle, &bs);
  if (err == ESP_ERR_NOT_FOUND) {
    bs.valuestring = "undefined";
    settings_set(&settings_handle, &bs);
  }

  bs.type = NUMBER;
  bs.valuestring = NULL;

  // BOARD TYPE
  bs.key = "board_type";
  err = settings_get(&settings_handle, &bs);
  if (err == ESP_ERR_NOT_FOUND) {
    bs.valueint = -1;
    settings_set(&settings_handle, &bs);
  }

  // Board types definition:
  // 0 = 433 MHz (downlink)
  // 1 = 915 MHz (uplink)
  // 2 = 433 MHz and 915 MHz (downlink and uplink)

  // LED PIN
  bs.key = "board_led";
  err = settings_get(&settings_handle, &bs);
  if (err == ESP_ERR_NOT_FOUND) {
    bs.valueint = -1;
    settings_set(&settings_handle, &bs);
  }

  // OLED SDA PIN
  bs.key = "oled_sda";
  err = settings_get(&settings_handle, &bs);
  if (err == ESP_ERR_NOT_FOUND) {
    bs.valueint = -1;
    settings_set(&settings_handle, &bs);
  }
  // OLED SCL PIN
  bs.key = "oled_scl";
  err = settings_get(&settings_handle, &bs);
  if (err == ESP_ERR_NOT_FOUND) {
    bs.valueint = -1;
    settings_set(&settings_handle, &bs);
  }
  // OLED RST PIN
  bs.key = "oled_rst";
  err = settings_get(&settings_handle, &bs);
  if (err == ESP_ERR_NOT_FOUND) {
    bs.valueint = -1;
    settings_set(&settings_handle, &bs);
  }

  // LoRa SCK PIN
  bs.key = "lora_sck";
  err = settings_get(&settings_handle, &bs);
  if (err == ESP_ERR_NOT_FOUND) {
    bs.valueint = -1;
    settings_set(&settings_handle, &bs);
  }
  // LoRa SDI (MISO) PIN
  bs.key = "lora_sdi";
  err = settings_get(&settings_handle, &bs);
  if (err == ESP_ERR_NOT_FOUND) {
    bs.valueint = -1;
    settings_set(&settings_handle, &bs);
  }
  // LoRa SDO (MOSI) PIN
  bs.key = "lora_sdo";
  err = settings_get(&settings_handle, &bs);
  if (err == ESP_ERR_NOT_FOUND) {
    bs.valueint = -1;
    settings_set(&settings_handle, &bs);
  }
  // LoRa CS (SS) PIN
  bs.key = "lora_cs";
  err = settings_get(&settings_handle, &bs);
  if (err == ESP_ERR_NOT_FOUND) {
    bs.valueint = -1;
    settings_set(&settings_handle, &bs);
  }
  // LoRa RST PIN
  bs.key = "lora_rst";
  err = settings_get(&settings_handle, &bs);
  if (err == ESP_ERR_NOT_FOUND) {
    bs.valueint = -1;
    settings_set(&settings_handle, &bs);
  }
  // LoRa DIO0 PIN
  bs.key = "lora_dio0";
  err = settings_get(&settings_handle, &bs);
  if (err == ESP_ERR_NOT_FOUND) {
    bs.valueint = -1;
    settings_set(&settings_handle, &bs);
  }

  printf("BOARD SETTINGS\n");
  settings_list();

  /* Prepare serial console for REPL */
  esp_console_repl_t *repl = NULL;
  esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
  repl_config.task_stack_size = (1024 * 8);
  repl_config.prompt = ">";

  /* Register commands */
  esp_console_register_help_command();
  register_settings();
  register_wifi();
  register_oled();
  register_ota();
  register_api();
  register_lora();
  // register_motors();

  initialize_wifi();
  initialize_oled();

  wifi_network_t network = {};

  setting_t wifi_ssid = {
    .key = "wifi_ssid"
  };
  err = settings_get(&settings_handle, &wifi_ssid);

  if (!wifi_ssid.valuestring) {
    printf("ssid not found\n");
  } else {
    printf("ssid: %s\n", wifi_ssid.valuestring);
    network.ssid = wifi_ssid.valuestring;
  }

  setting_t wifi_pass = {
    .key = "wifi_pass"
  };
  err = settings_get(&settings_handle, &wifi_pass);

  if (!wifi_pass.valuestring) {
    printf("password not found\n");
  } else {
    printf("password: %s\n", wifi_pass.valuestring);
    network.password = wifi_pass.valuestring;
  }

  if (!network.ssid || !network.password) {
    printf("no wifi credentials found\n");
  } else {
    join_wifi(&network);
  }

  settings_free(&settings_handle);

  /* Setup console REPL over UART */
  esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));

  /* Run REPL */
  ESP_ERROR_CHECK(esp_console_start_repl(repl));
}
