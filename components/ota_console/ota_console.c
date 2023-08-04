#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "ota.h"
#include "ota_console.h"

static const char* TAG = "OTA_CONSOLE";
// #define LOG_LOCAL_LEVEL ESP_LOG_WARN

void register_ota() {
    ESP_LOGI(TAG, "register_ota");
}