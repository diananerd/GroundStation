#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "esp_ota_ops.h"
#include "nvs_flash.h"
#include "settings.h"

static const char* TAG = "GroundStation";

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
    .key = "foo",
    .valuestring = "updated as struct"
  };

  setting_t bs_str_out = {
    .key = "foo"
  };

  err = settings_set(&settings_handle, &bs_str);
  err = settings_get(&settings_handle, &bs_str_out);
  ESP_LOGI(TAG, "FINAL type %s %s=%s", SETTING_TYPE_NAMES[bs_str_out.type], bs_str_out.key, bs_str_out.valuestring);

  setting_t bs_int = {
    .type = NUMBER,
    .key = "count",
    .valueint = 1024
  };

  setting_t bs_int_out = {
    .key = "count"
  };

  err = settings_set(&settings_handle, &bs_int);
  err = settings_get(&settings_handle, &bs_int_out);
  ESP_LOGI(TAG, "FINAL type %s %s=%i", SETTING_TYPE_NAMES[bs_int_out.type], bs_int_out.key, bs_int_out.valueint);

  // err = settings_get_str(&settings_handle, "foo", &foo);
  // ESP_LOGI(TAG, "A) settings_get foo=%s", foo);
  // err = settings_set_str(&settings_handle, "foo", "alt value");
  // err = settings_get_str(&settings_handle, "foo", &foo);
  // ESP_LOGI(TAG, "B) settings_get foo=%s", foo);

  // err = settings_get_int(&settings_handle, "count", &mycount);
  // ESP_LOGI(TAG, "C) settings_get count=%i", mycount);
  // err = settings_set_int(&settings_handle, "count", 14);
  // err = settings_get_int(&settings_handle, "count", &mycount);
  // ESP_LOGI(TAG, "D) settings_get count=%i", mycount);

  settings_free(&settings_handle);

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

  /* Setup console REPL over UART */
  esp_console_dev_uart_config_t hw_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_console_new_repl_uart(&hw_config, &repl_config, &repl));

  /* Run REPL */
  ESP_ERROR_CHECK(esp_console_start_repl(repl));
}
