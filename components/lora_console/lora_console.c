#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "lora.h"
#include "lora_console.h"

static const char* TAG = "LORA_CONSOLE";
// #define LOG_LOCAL_LEVEL ESP_LOG_WARN

static struct {
    struct arg_end *end;
} send_lora_args;

static int send_lora_exec(int argc, char **argv) {
    ESP_LOGI(TAG, "send_lora_exec");
    int nerrors = arg_parse(argc, argv, (void **) &send_lora_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, send_lora_args.end, argv[0]);
        return 1;
    }
    return 0;
}

static struct {
    struct arg_end *end;
} test_lora_args;

static int test_lora_exec(int argc, char **argv) {
    ESP_LOGI(TAG, "test_lora_exec");
    int nerrors = arg_parse(argc, argv, (void **) &test_lora_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, test_lora_args.end, argv[0]);
        return 1;
    }
    return 0;
}

static struct {
    struct arg_end *end;
} listen_lora_args;

static int listen_lora_exec(int argc, char **argv) {
    ESP_LOGI(TAG, "listen_lora_exec");
    int nerrors = arg_parse(argc, argv, (void **) &listen_lora_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, listen_lora_args.end, argv[0]);
        return 1;
    }
    return 0;
}

void register_lora() {
    ESP_LOGI(TAG, "register_lora");

    send_lora_args.end = arg_end(1);

    const esp_console_cmd_t send_lora_cmd = {
        .command = "send",
        .help = "Send message via LoRa",
        .hint = NULL,
        .func = &send_lora_exec,
        .argtable = &send_lora_args
    };

    test_lora_args.end = arg_end(1);

    const esp_console_cmd_t test_lora_cmd = {
        .command = "test",
        .help = "Send LoRa ground station test packages",
        .hint = NULL,
        .func = &test_lora_exec,
        .argtable = &test_lora_args
    };

    listen_lora_args.end = arg_end(1);

    const esp_console_cmd_t listen_lora_cmd = {
        .command = "listen",
        .help = "Listen for LoRa packages and show in console for debugging purposes",
        .hint = NULL,
        .func = &listen_lora_exec,
        .argtable = &listen_lora_args
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&send_lora_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&test_lora_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&listen_lora_cmd));
}