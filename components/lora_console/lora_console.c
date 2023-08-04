#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "lora.h"
#include "lora_console.h"

static const char* TAG = "LORA_CONSOLE";
// #define LOG_LOCAL_LEVEL ESP_LOG_WARN

void register_lora() {
    ESP_LOGI(TAG, "register_lora");
}