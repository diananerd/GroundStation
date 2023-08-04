#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "motors.h"
#include "motors_console.h"

static const char* TAG = "MOTORS_CONSOLE";
// #define LOG_LOCAL_LEVEL ESP_LOG_WARN

static struct {
    struct arg_end *end;
} azimuth_motors_args;

static int azimuth_motors_exec(int argc, char **argv) {
    ESP_LOGI(TAG, "azimuth_motors_exec");
    int nerrors = arg_parse(argc, argv, (void **) &azimuth_motors_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, azimuth_motors_args.end, argv[0]);
        return 1;
    }
    return 0;
}

static struct {
    struct arg_end *end;
} elevation_motors_args;

static int elevation_motors_exec(int argc, char **argv) {
    ESP_LOGI(TAG, "elevation_motors_exec");
    int nerrors = arg_parse(argc, argv, (void **) &elevation_motors_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, elevation_motors_args.end, argv[0]);
        return 1;
    }
    return 0;
}

static struct {
    struct arg_end *end;
} lookzero_motors_args;

static int lookzero_motors_exec(int argc, char **argv) {
    ESP_LOGI(TAG, "lookzero_motors_exec");
    int nerrors = arg_parse(argc, argv, (void **) &lookzero_motors_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, lookzero_motors_args.end, argv[0]);
        return 1;
    }
    return 0;
}

void register_motors() {
    ESP_LOGI(TAG, "register_motors");

    azimuth_motors_args.end = arg_end(1);

    const esp_console_cmd_t azimuth_motors_cmd = {
        .command = "azimuth",
        .help = "Set azimuth angle in degrees",
        .hint = NULL,
        .func = &azimuth_motors_exec,
        .argtable = &azimuth_motors_args
    };

    elevation_motors_args.end = arg_end(1);

    const esp_console_cmd_t elevation_motors_cmd = {
        .command = "elevation",
        .help = "Set elevation angle in degrees",
        .hint = NULL,
        .func = &elevation_motors_exec,
        .argtable = &elevation_motors_args
    };

    lookzero_motors_args.end = arg_end(1);

    const esp_console_cmd_t lookzero_motors_cmd = {
        .command = "lookzero",
        .help = "Set both angles to zero (for calibration)",
        .hint = NULL,
        .func = &lookzero_motors_exec,
        .argtable = &lookzero_motors_args
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&azimuth_motors_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&elevation_motors_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&lookzero_motors_cmd));
}