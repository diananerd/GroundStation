#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "ota.h"
#include "ota_console.h"

static const char* TAG = "OTA_CONSOLE";
// #define LOG_LOCAL_LEVEL ESP_LOG_WARN

static struct {
    struct arg_end *end;
} updates_ota_args;

static int updates_ota_exec(int argc, char **argv) {
    ESP_LOGI(TAG, "updates_ota_exec");
    int nerrors = arg_parse(argc, argv, (void **) &updates_ota_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, updates_ota_args.end, argv[0]);
        return 1;
    }
    return 0;
}

static struct {
    struct arg_end *end;
} update_ota_args;

static int update_ota_exec(int argc, char **argv) {
    ESP_LOGI(TAG, "update_ota_exec");
    int nerrors = arg_parse(argc, argv, (void **) &update_ota_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, update_ota_args.end, argv[0]);
        return 1;
    }
    return 0;
}

void register_ota() {
    ESP_LOGI(TAG, "register_ota");

    updates_ota_args.end = arg_end(1);

    const esp_console_cmd_t updates_ota_cmd = {
        .command = "updates",
        .help = "List firmware versions and if current is up-to-date",
        .hint = NULL,
        .func = &updates_ota_exec,
        .argtable = &updates_ota_args
    };

    update_ota_args.end = arg_end(1);

    const esp_console_cmd_t update_ota_cmd = {
        .command = "update",
        .help = "Manually update firmware to last stable version or pass custom version as argument",
        .hint = NULL,
        .func = &update_ota_exec,
        .argtable = &update_ota_args
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&updates_ota_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&update_ota_cmd));
}