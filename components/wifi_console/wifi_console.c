#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "wifi.h"
#include "wifi_console.h"

static const char* TAG = "WIFI_CONSOLE";
// #define LOG_LOCAL_LEVEL ESP_LOG_WARN

void register_wifi() {
    ESP_LOGI(TAG, "register_wifi");
}