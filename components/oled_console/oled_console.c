#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "oled.h"
#include "oled_console.h"

static const char* TAG = "OLED_CONSOLE";
// #define LOG_LOCAL_LEVEL ESP_LOG_WARN

void register_oled() {
    ESP_LOGI(TAG, "register_oled");
}