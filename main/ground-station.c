#include <string.h>
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
  err = settings_create(&settings_handle);

  setting_t bs_str = {
    .type = STRING,
    .key = "board_name",
    .valuestring = "TTGO LoRa32 v2 433 MHz"
  };

  err = settings_set(&settings_handle, &bs_str);

  setting_t wifi_ssid = {
    .key = "wifi_ssid"
  };

  err = settings_get(&settings_handle, &wifi_ssid);

  if (!wifi_ssid.valuestring) {
    printf("ssid not found\n");
  } else {
    printf("ssid: %s\n", wifi_ssid.valuestring);
  }

  setting_t wifi_pass = {
    .key = "wifi_pass"
  };

  err = settings_get(&settings_handle, &wifi_pass);

  if (!wifi_pass.valuestring) {
    printf("password not found\n");
  } else {
    int password_len = strlen(wifi_pass.valuestring);
    int password_end = 4;
    if (password_len < 8) {
      printf("password too short to display\n");
    } else {
      char* password = wifi_pass.valuestring;
      for (int i = 0; i < password_len - password_end; i++) {
        password[i] = '*';
      }
      printf("password: %s\n", password);
    }
  }

  // setting_t bs_int = {
  //   .type = NUMBER,
  //   .key = "type",
  //   .valueint = 0
  // };

  // setting_t bs_int_out = {
  //   .key = "type"
  // };

  // err = settings_set(&settings_handle, &bs_int);
  // err = settings_get(&settings_handle, &bs_int_out);
  // ESP_LOGI(TAG, "SET %s %s=%i", SETTING_TYPE_NAMES[bs_int_out.type], bs_int_out.key, bs_int_out.valueint);

  // settings_free(&settings_handle);

  // printf("BOARD SETTINGS\n");
  // settings_list();

  /* Prepare serial console for REPL */
  esp_console_repl_t *repl = NULL;
  esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
  repl_config.task_stack_size = (1024 * 8);
  repl_config.prompt = ">";

  // board_settings_t bs = {
  //   .name = "TTGO_LoRA_v2_433MHz",
  //   .type = 0,
  //   .aADDR = 60,
  //   .oSDA = 21,
  //   .oSCL = 22,
  //   .oRST = 16,
  //   .pBut = 0,
  //   .led = 22,
  //   .lNSS = 18,
  //   .lDIO0 = 26,
  //   .lDIO1 = 33,
  //   .lBUSSY = 0,
  //   .lRST = 14,
  //   .lMISO = 19,
  //   .lMOSI = 27,
  //   .lSCK = 5
  // };

  /* Register commands */
  esp_console_register_help_command();
  register_settings();
  register_wifi();
  register_oled();
  register_ota();
  register_api();
  register_lora();
  register_motors();

  /* Setup console REPL over UART */
  esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));

  /* Run REPL */
  ESP_ERROR_CHECK(esp_console_start_repl(repl));
}
