#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "oled.h"
#include "oled_console.h"

static const char* TAG = "OLED_CONSOLE";
// #define LOG_LOCAL_LEVEL ESP_LOG_WARN

static struct {
    struct arg_end *end;
} print_oled_args;

static int print_oled_exec(int argc, char **argv) {
    ESP_LOGI(TAG, "print_oled_exec");
    int nerrors = arg_parse(argc, argv, (void **) &print_oled_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, print_oled_args.end, argv[0]);
        return 1;
    }
    return 0;
}

static struct {
    struct arg_end *end;
} clean_oled_args;

static int clean_oled_exec(int argc, char **argv) {
    ESP_LOGI(TAG, "clean_oled_exec");
    int nerrors = arg_parse(argc, argv, (void **) &clean_oled_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, clean_oled_args.end, argv[0]);
        return 1;
    }
    return 0;
}

static struct {
    struct arg_end *end;
} version_oled_args;

static int version_oled_exec(int argc, char **argv) {
    ESP_LOGI(TAG, "version_oled_exec");
    int nerrors = arg_parse(argc, argv, (void **) &version_oled_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, version_oled_args.end, argv[0]);
        return 1;
    }
    return 0;
}

void register_oled() {
    ESP_LOGI(TAG, "register_oled");

    print_oled_args.end = arg_end(1);

    const esp_console_cmd_t print_oled_cmd = {
        .command = "print",
        .help = "Print text to OLED screen in logs mode with autoscroll",
        .hint = NULL,
        .func = &print_oled_exec,
        .argtable = &print_oled_args
    };

    clean_oled_args.end = arg_end(1);

    const esp_console_cmd_t clean_oled_cmd = {
        .command = "clean",
        .help = "Clean OLED screen",
        .hint = NULL,
        .func = &clean_oled_exec,
        .argtable = &clean_oled_args
    };

    version_oled_args.end = arg_end(1);

    const esp_console_cmd_t version_oled_cmd = {
        .command = "version",
        .help = "Show firmware version in OLED screen and in console",
        .hint = NULL,
        .func = &version_oled_exec,
        .argtable = &version_oled_args
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&print_oled_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&clean_oled_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&version_oled_cmd));
}