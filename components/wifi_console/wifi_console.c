#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "wifi.h"
#include "wifi_console.h"

static const char* TAG = "WIFI_CONSOLE";
// #define LOG_LOCAL_LEVEL ESP_LOG_WARN

static struct {
    struct arg_end *end;
} networks_wifi_args;

static int networks_wifi_exec(int argc, char **argv) {
    ESP_LOGI(TAG, "networks_wifi_exec");
    int nerrors = arg_parse(argc, argv, (void **) &networks_wifi_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, networks_wifi_args.end, argv[0]);
        return 1;
    }
    return 0;
}

static struct {
    struct arg_end *end;
} join_wifi_args;

static int join_wifi_exec(int argc, char **argv) {
    ESP_LOGI(TAG, "join_wifi_exec");
    int nerrors = arg_parse(argc, argv, (void **) &join_wifi_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, join_wifi_args.end, argv[0]);
        return 1;
    }
    return 0;
}

static struct {
    struct arg_end *end;
} netstat_wifi_args;

static int netstat_wifi_exec(int argc, char **argv) {
    ESP_LOGI(TAG, "netstat_wifi_exec");
    int nerrors = arg_parse(argc, argv, (void **) &netstat_wifi_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, netstat_wifi_args.end, argv[0]);
        return 1;
    }
    return 0;
}

static struct {
    struct arg_end *end;
} ip_wifi_args;

static int ip_wifi_exec(int argc, char **argv) {
    ESP_LOGI(TAG, "ip_wifi_exec");
    int nerrors = arg_parse(argc, argv, (void **) &ip_wifi_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, ip_wifi_args.end, argv[0]);
        return 1;
    }
    return 0;
}

void register_wifi() {
    ESP_LOGI(TAG, "register_wifi");

    networks_wifi_args.end = arg_end(1);

    const esp_console_cmd_t networks_wifi_cmd = {
        .command = "networks",
        .help = "List available networks",
        .hint = NULL,
        .func = &networks_wifi_exec,
        .argtable = &networks_wifi_args
    };

    join_wifi_args.end = arg_end(1);

    const esp_console_cmd_t join_wifi_cmd = {
        .command = "join",
        .help = "Join to network with ssid and password",
        .hint = NULL,
        .func = &join_wifi_exec,
        .argtable = &join_wifi_args
    };

    netstat_wifi_args.end = arg_end(1);

    const esp_console_cmd_t netstat_wifi_cmd = {
        .command = "netstat",
        .help = "Show network status (is online)",
        .hint = NULL,
        .func = &netstat_wifi_exec,
        .argtable = &netstat_wifi_args
    };

    ip_wifi_args.end = arg_end(1);

    const esp_console_cmd_t ip_wifi_cmd = {
        .command = "ip",
        .help = "Show local network IP",
        .hint = NULL,
        .func = &ip_wifi_exec,
        .argtable = &ip_wifi_args
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&networks_wifi_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&join_wifi_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&netstat_wifi_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&ip_wifi_cmd));
}