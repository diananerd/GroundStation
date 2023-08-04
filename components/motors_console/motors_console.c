#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "motors.h"
#include "motors_console.h"

static const char* TAG = "MOTORS_CONSOLE";
// #define LOG_LOCAL_LEVEL ESP_LOG_WARN

void register_motors() {
    ESP_LOGI(TAG, "register_motors");
}