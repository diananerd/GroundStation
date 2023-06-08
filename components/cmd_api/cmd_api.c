#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "api_calls.h"

void initialize_api(void) {
    ESP_LOGI("api", "initialize api");
}

static struct {
    struct arg_int *timeout;
    struct arg_str *url;
    struct arg_end *end;
} ping_args;

static int ping(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &ping_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, ping_args.end, argv[0]);
        return 1;
    }
    ESP_LOGI(__func__, "Ping to '%s'", ping_args.url->sval[0]);
    bool err = ping_url(ping_args.url->sval[0], ping_args.timeout->ival[0]);
    if (err) {
        ESP_LOGW(__func__, "Connection timed out");
        return 1;
    }
    ESP_LOGI(__func__, "Ping end");
    return 0;
}

static struct {
    struct arg_int *timeout;
    struct arg_end *end;
} sync_args;

static int sync_handler(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &sync_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, sync_args.end, argv[0]);
        return 1;
    }
    ESP_LOGI(__func__, "Sync account");
    bool err = sync_account();
    if (err) {
        ESP_LOGW(__func__, "Connection timed out");
        return 1;
    }
    ESP_LOGI(__func__, "Sync end");
    return 0;
}

void register_api(void) {
    ping_args.url = arg_str1(NULL, NULL, "<url>", "Site url");
    ping_args.timeout = arg_int0("t", "timeout", "<t>", "Connection timeout, ms");
    ping_args.end = arg_end(2);

    const esp_console_cmd_t ping_cmd = {
        .command = "ping",
        .help = "Ping website",
        .hint = NULL,
        .func = &ping,
        .argtable = &ping_args
    };

    sync_args.timeout = arg_int0("t", "timeout", "<t>", "Connection timeout, ms");
    sync_args.end = arg_end(2);

    const esp_console_cmd_t sync_cmd = {
        .command = "sync",
        .help = "Sync account",
        .hint = NULL,
        .func = &sync_handler,
        .argtable = &sync_args
    };

    ESP_ERROR_CHECK( esp_console_cmd_register(&ping_cmd) );
    ESP_ERROR_CHECK( esp_console_cmd_register(&sync_cmd) );
}
