#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "api.h"
#include "api_console.h"

static const char* TAG = "API_CONSOLE";
// #define LOG_LOCAL_LEVEL ESP_LOG_WARN

void register_api() {
    ESP_LOGI(TAG, "register_api");
}