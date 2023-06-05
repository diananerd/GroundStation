/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* Console example — WiFi commands

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "cmd_wifi.h"

#define JOIN_TIMEOUT_MS (10000)
#define LIST_TIMEOUT_MS (10000)
#define AP_RECORDS_LIST_SIZE 20
#define WAIT_FOR_AP_SCAN false

static EventGroupHandle_t wifi_event_group;
const int SCAN_BIT = BIT0;
const int CONNECTED_BIT = BIT1;
static bool initialized = false;

static void wifi_scan_show_records() {
    for (;;) {
        uint16_t number = AP_RECORDS_LIST_SIZE;
        uint16_t count_ap = 0;
        wifi_ap_record_t ap_records[AP_RECORDS_LIST_SIZE];
        memset(ap_records, 0, sizeof(ap_records));

        ESP_ERROR_CHECK( esp_wifi_scan_get_ap_num(&count_ap) );
        ESP_ERROR_CHECK( esp_wifi_scan_get_ap_records(&number, ap_records) );

        for (uint16_t i = 0; i < AP_RECORDS_LIST_SIZE && i < count_ap; i++) {
            char *ssid = (char *) ap_records[i].ssid;
            // ESP_LOGI("network", "%i %s", i, ssid);
            printf("%i) %s\n", i + 1, ssid);
        }

        xEventGroupSetBits(wifi_event_group, SCAN_BIT);
        vTaskDelete(NULL);
    }
}

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_SCAN_DONE) {
        xTaskCreate(wifi_scan_show_records, "show_networks", 1024 * 32, NULL, 10, NULL);
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        printf("WiFi Connected\n");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
    }
}

void initialise_wifi(void)
{
    ESP_LOGI("wifi", "initialize wifi");
    // esp_log_level_set("wifi", ESP_LOG_WARN);
    if (initialized) {
        ESP_LOGI("wifi", "previously initialized");
        return;
    }
    ESP_LOGI("wifi", "initializing...");
    ESP_ERROR_CHECK(esp_netif_init());
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);
    ESP_LOGI("wifi", "set config");
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_LOGI("wifi", "add event handlers");
    ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_SCAN_DONE, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL) );
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_LOGI("wifi", "start");
    ESP_ERROR_CHECK( esp_wifi_start() );
    initialized = true;
    ESP_LOGI("wifi", "connect");
    esp_err_t err = esp_wifi_connect();
    if (err == ESP_OK) {
        ESP_LOGI("join", "WiFi connected");
    } else if (err == ESP_ERR_WIFI_NOT_INIT) {
        ESP_LOGE("join", "WiFi not initialized");
    } else if (err == ESP_ERR_WIFI_NOT_STARTED) {
        ESP_LOGE("join", "WiFi not started");
    } else if (err == ESP_ERR_WIFI_CONN) {
        ESP_LOGE("join", "WiFi internal error");
    } else if (err == ESP_ERR_WIFI_SSID) {
        ESP_LOGE("join", "WiFi invalid ssid");
    }
    ESP_LOGI("wifi", "initialize end");
}

static bool wifi_list(int timeout_ms)
{
    ESP_LOGI("networks", "scan start");
    wifi_scan_config_t wifi_scan_config = {};
    esp_wifi_scan_start(&wifi_scan_config, WAIT_FOR_AP_SCAN);

    int bits = xEventGroupWaitBits(wifi_event_group, SCAN_BIT,
                                   pdFALSE, pdTRUE, timeout_ms / portTICK_PERIOD_MS);

    ESP_LOGI("networks", "scan stop");
    esp_wifi_scan_stop();
    xEventGroupClearBits(wifi_event_group, SCAN_BIT);

    return (bits & SCAN_BIT) != 0;
}

/** Arguments used by 'networks' function */
static struct {
    struct arg_int *timeout;
    struct arg_end *end;
} networks_args;

static int networks(int argc, char **argv) {
    if (!initialized) {
        ESP_LOGE(__func__, "WiFi not initialized");
        return 1;
    }

    int nerrors = arg_parse(argc, argv, (void **) &networks_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, networks_args.end, argv[0]);
        return 1;
    }
    ESP_LOGI(__func__, "List networks");

    /* set default value*/
    if (networks_args.timeout->count == 0) {
        networks_args.timeout->ival[0] = LIST_TIMEOUT_MS;
    }

    bool listed = wifi_list(networks_args.timeout->ival[0]);
    if (!listed) {
        ESP_LOGW(__func__, "List networks timed out");
        return 1;
    }
    return 0;
}

static bool wifi_join(const char *ssid, const char *pass, int timeout_ms)
{
    wifi_config_t wifi_config = { 0 };
    strlcpy((char *) wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    if (pass) {
        strlcpy((char *) wifi_config.sta.password, pass, sizeof(wifi_config.sta.password));
    }

    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    esp_err_t err = esp_wifi_connect();

    if (err == ESP_OK) {
        ESP_LOGI("join", "WiFi connected");
    } else if (err == ESP_ERR_WIFI_NOT_INIT) {
        ESP_LOGE("join", "WiFi not initialized");
    } else if (err == ESP_ERR_WIFI_NOT_STARTED) {
        ESP_LOGE("join", "WiFi not started");
    } else if (err == ESP_ERR_WIFI_CONN) {
        ESP_LOGE("join", "WiFi internal error");
    } else if (err == ESP_ERR_WIFI_SSID) {
        ESP_LOGE("join", "WiFi invalid ssid");
    }

    int bits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                                   pdFALSE, pdTRUE, timeout_ms / portTICK_PERIOD_MS);
    return (bits & CONNECTED_BIT) != 0;
}

/** Arguments used by 'join' function */
static struct {
    struct arg_int *timeout;
    struct arg_str *ssid;
    struct arg_str *password;
    struct arg_end *end;
} join_args;

static int connect(int argc, char **argv)
{
    if (!initialized) {
        ESP_LOGE(__func__, "WiFi not initialized");
        return 1;
    }

    int nerrors = arg_parse(argc, argv, (void **) &join_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, join_args.end, argv[0]);
        return 1;
    }
    ESP_LOGI(__func__, "Connecting to '%s'",
             join_args.ssid->sval[0]);

    /* set default value*/
    if (join_args.timeout->count == 0) {
        join_args.timeout->ival[0] = JOIN_TIMEOUT_MS;
    }

    bool connected = wifi_join(join_args.ssid->sval[0],
                               join_args.password->sval[0],
                               join_args.timeout->ival[0]);
    if (!connected) {
        ESP_LOGW(__func__, "Connection timed out");
        return 1;
    }
    ESP_LOGI(__func__, "Connected");
    return 0;
}

static bool network_status()
{
    int bits = xEventGroupGetBits(wifi_event_group);
    bool connected = (bits & CONNECTED_BIT) != 0;
    if (connected) {
        printf("Network connected\n");
    } else {
        printf("Network disconnected\n");
    }
    return 0;
}

/** Arguments used by 'networks' function */
static struct {
    struct arg_int *timeout;
    struct arg_end *end;
} status_args;

static int status(int argc, char **argv)
{
    if (!initialized) {
        ESP_LOGE(__func__, "Network not initialized");
        return 1;
    }

    int nerrors = arg_parse(argc, argv, (void **) &status_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, status_args.end, argv[0]);
        return 1;
    }
    ESP_LOGI(__func__, "Load network status");

    bool err = network_status();
    if (err) {
        ESP_LOGW(__func__, "Failed status");
        return 1;
    }
    ESP_LOGI(__func__, "Success status");
    return 0;
}

void register_wifi(void)
{
    status_args.timeout = arg_int0(NULL, "timeout", "<t>", "Connection timeout, ms");
    status_args.end = arg_end(2);

    const esp_console_cmd_t status_cmd = {
        .command = "status",
        .help = "Show network status",
        .hint = NULL,
        .func = &status,
        .argtable = &status_args
    };

    networks_args.timeout = arg_int0(NULL, "timeout", "<t>", "Connection timeout, ms");
    networks_args.end = arg_end(2);

    const esp_console_cmd_t networks_cmd = {
        .command = "networks",
        .help = "List WiFi networks",
        .hint = NULL,
        .func = &networks,
        .argtable = &networks_args
    };

    join_args.timeout = arg_int0(NULL, "timeout", "<t>", "Connection timeout, ms");
    join_args.ssid = arg_str1(NULL, NULL, "<ssid>", "SSID of AP");
    join_args.password = arg_str0(NULL, NULL, "<pass>", "PSK of AP");
    join_args.end = arg_end(2);

    const esp_console_cmd_t join_cmd = {
        .command = "join",
        .help = "Join WiFi AP as a station",
        .hint = NULL,
        .func = &connect,
        .argtable = &join_args
    };

    ESP_ERROR_CHECK( esp_console_cmd_register(&join_cmd) );
    ESP_ERROR_CHECK( esp_console_cmd_register(&networks_cmd) );
    ESP_ERROR_CHECK( esp_console_cmd_register(&status_cmd) );
}
