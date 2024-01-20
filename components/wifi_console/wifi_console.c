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
    struct arg_lit *json;
    struct arg_end *end;
} networks_wifi_args;

static int networks_wifi_exec(int argc, char **argv) {
    ESP_LOGI(TAG, "networks_wifi_exec");
    int nerrors = arg_parse(argc, argv, (void **) &networks_wifi_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, networks_wifi_args.end, argv[0]);
        return 1;
    }

    wifi_networks_t networks = {};
    esp_err_t err = networks_wifi(&networks);

    if (networks_wifi_args.json->count > 0) {
            if (err == ESP_ERR_WIFI_TIMEOUT) {
            ESP_LOGI(TAG, "timeout handled");
            printf("{\"error\":\"timeout error\"}\n");
            return 0;
        } else if (err != ESP_OK) {
            ESP_LOGE(TAG, "%s", esp_err_to_name(err));
            printf("{\"error\":\"connection failed\"}\n");
            return 0;
        }

        if (networks.size == 0) {
            printf("{\"error\":\"not found\"}\n");
            return 0;
        }

        int final_size = 1; // Add 1 for string end character
        for (int i = 0; i < networks.size; i++) {
            wifi_network_t network = networks.list[i];
            final_size += strlen(network.ssid);
        }
        // Add (2(quote marks per network) * n) + (n(commas) - 1)
        // n-commas - 1; excludes trailing comma
        final_size += (networks.size * 2) + networks.size - 1;
        char* output = (char*)malloc(final_size);
        if (output == NULL) {
            ESP_LOGE(TAG, "error in memory allocation");
            printf("undefined error\n");
            return 0;
        }
        strcpy(output, "");
        for (int i = 0; i < networks.size; i++) {
            wifi_network_t network = networks.list[i];
            char* comma = (i != networks.size -1) ? "," : "";
            char* net = (char*)malloc(strlen((char*)network.ssid) + strlen(comma) + 3); // 2 quotes + 1 string end
            sprintf(net, "\"%s\"%s", network.ssid, comma);
            strncat(output, net, final_size);
            free(net);
        }
        output[final_size - 1] = '\0';
        printf("{\"networks\":[%s]}\n", output);
        free(output);

    } else {
        if (err == ESP_ERR_WIFI_TIMEOUT) {
            ESP_LOGI(TAG, "timeout handled");
            printf("timeout error\n");
            return 0;
        } else if (err != ESP_OK) {
            ESP_LOGE(TAG, "%s", esp_err_to_name(err));
            printf("connection failed\n");
            return 0;
        }

        if (networks.size == 0) {
            printf("not found\n");
            return 0;
        }

        for (int i = 0; i < networks.size; i++) {
            wifi_network_t network = networks.list[i];
            printf("%d) %s\n", i + 1, (char*)network.ssid);
        }
    }

    networks_free(&networks);
    return 0;
}

static struct {
    struct arg_str *ssid;
    struct arg_str *password;
    struct arg_lit *json;
    struct arg_end *end;
} join_wifi_args;

static int join_wifi_exec(int argc, char **argv) {
    ESP_LOGI(TAG, "join_wifi_exec");
    int nerrors = arg_parse(argc, argv, (void **) &join_wifi_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, join_wifi_args.end, argv[0]);
        return 1;
    }

    if (join_wifi_args.ssid->count <= 0) {
        ESP_LOGE(TAG, "ssid not found");
        if (join_wifi_args.json->count > 0) {
            printf("{\"error\":\"ssid not found\"}\n");
        } else {
            printf("ssid not found\n");
        }
        return 0;
    }

    if (join_wifi_args.password->count <= 0) {
        ESP_LOGE(TAG, "password not found");
        if (join_wifi_args.json->count > 0) {
            printf("{\"error\":\"password not found\"}\n");
        } else {
            printf("password not found\n");
        }
        return 0;
    }

    wifi_network_t network = {
        .ssid = join_wifi_args.ssid->sval[0],
        .password = join_wifi_args.password->sval[0]
    };

    esp_err_t err = join_wifi(&network);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s", esp_err_to_name(err));
        return err;
    }

    wifi_netstat_t netstat = {};
    err = netstat_wifi(&netstat);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s", esp_err_to_name(err));
        return err;
    }

    if (join_wifi_args.json->count > 0) {
        if (netstat.connected) {
            printf("{\"connection\":\"success\"}\n");
        } else {
            printf("{\"error\":\"connection failed\"}\n");
        }
    } else {
        if (netstat.connected) {
            printf("connection success\n");
        } else {
            printf("connection failed\n");
        }
    }

    return 0;
}

static struct {
    struct arg_lit *status;
    struct arg_lit *network;
    struct arg_lit *ip;
    struct arg_lit *json;
    struct arg_end *end;
} netstat_wifi_args;

static int netstat_wifi_exec(int argc, char **argv) {
    ESP_LOGI(TAG, "netstat_wifi_exec");
    int nerrors = arg_parse(argc, argv, (void **) &netstat_wifi_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, netstat_wifi_args.end, argv[0]);
        return 1;
    }

    wifi_netstat_t netstat = {};
    esp_err_t err = netstat_wifi(&netstat);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s", esp_err_to_name(err));
        return err;
    }

    if (netstat_wifi_args.status->count > 0) {
        printf("%s\n", CONNECTION_STATE_NAMES[netstat.status]);
    } else if (netstat_wifi_args.network->count > 0) {
        printf("%s\n", strlen(netstat.network) == 0 ? "unknown" : netstat.network);
    } else if (netstat_wifi_args.ip->count > 0) {
        printf("%s\n", strlen(netstat.ip) == 0 ? "unknown" : netstat.ip);
    } else if (netstat_wifi_args.json->count > 0) {
        printf(
            "{\"status\": \"%s\",\"network\":\"%s\",\"ip\":\"%s\"}\n",
            CONNECTION_STATE_NAMES[netstat.status],
            strlen(netstat.network) == 0 ? "unknown" : netstat.network,
            strlen(netstat.ip) == 0 ? "unknown" : netstat.ip
        );
    } else {
        printf("status: %s\n", CONNECTION_STATE_NAMES[netstat.status]);
        printf("network: %s\n", strlen(netstat.network) == 0 ? "unknown" : netstat.network);
        printf("ip: %s\n", strlen(netstat.ip) == 0 ? "unknown" : netstat.ip);
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

    networks_wifi_args.json = arg_lit0("j", "json", "show output as json");
    networks_wifi_args.end = arg_end(1);

    const esp_console_cmd_t networks_wifi_cmd = {
        .command = "networks",
        .help = "List available networks",
        .hint = NULL,
        .func = &networks_wifi_exec,
        .argtable = &networks_wifi_args
    };

    join_wifi_args.ssid = arg_str1(NULL, NULL, "ssid", "network ssid");
    join_wifi_args.password = arg_str0(NULL, NULL, "password", "netword password");
    join_wifi_args.json = arg_lit0("j", "json", "show output as json");
    join_wifi_args.end = arg_end(1);

    const esp_console_cmd_t join_wifi_cmd = {
        .command = "join",
        .help = "Join to network with ssid and password",
        .hint = NULL,
        .func = &join_wifi_exec,
        .argtable = &join_wifi_args
    };

    netstat_wifi_args.status = arg_lit0("s", "status", "return only connection status");
    netstat_wifi_args.network = arg_lit0("n", "network", "return only connected network ssid");
    netstat_wifi_args.ip = arg_lit0("a", "ip", "return only IP address");
    netstat_wifi_args.json = arg_lit0("j", "json", "show output as json");
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