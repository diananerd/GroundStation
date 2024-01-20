#include <stdio.h>
#include <string.h>
#include "math.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "lwip/netif.h"
#include "wifi.h"

#define MAX_WIFI_CONNECTION_RETRIES 5

static const char* TAG = "WIFI";
// #define LOG_LOCAL_LEVEL ESP_LOG_WARN

static int wifi_connection_retries = 0;
static wifi_status_t wifi_status = DISCONNECTED;
static char* ip = "";

static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "start event");
        esp_err_t err = esp_wifi_connect();
        wifi_status = CONNECTING;
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "%s", esp_err_to_name(err));
            wifi_status = DISCONNECTED;
            wifi_connection_retries = 0;
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG,"disconnected event");
        ip = "";
        if (wifi_connection_retries < MAX_WIFI_CONNECTION_RETRIES) {
            esp_wifi_connect();
            wifi_status = CONNECTING;
            wifi_connection_retries++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            ESP_LOGE(TAG, "connection failed");
            wifi_status = DISCONNECTED;
            wifi_connection_retries = 0;
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED) {
        ESP_LOGI(TAG,"connected event");
        wifi_status = CONNECTED;
        wifi_connection_retries = 0;
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        // ESP_LOGI(TAG, "got ip event:" IPSTR, IP2STR(&event->ip_info.ip));
        char local_ip[16];
        sprintf(local_ip, IPSTR, IP2STR(&event->ip_info.ip));
        ip = (char*)malloc(strlen(local_ip) + 1);
        strcpy(ip, local_ip);
    }
}

esp_err_t initialize_wifi() {
    ESP_LOGI(TAG, "initialize_wifi");

    esp_err_t err = ESP_OK;

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &event_handler,
            NULL,
            &instance_any_id
        )
    );

    ESP_ERROR_CHECK(
        esp_event_handler_instance_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &event_handler,
            NULL,
            &instance_got_ip
        )
    );

    wifi_config_t wifi_config = {};
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_get_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    err = esp_wifi_connect();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s", esp_err_to_name(err));
    }

    return err;
}

int networks_push(wifi_networks_t* networks, wifi_network_t network) {
    networks->size++;
    if (networks->size > 0) {
        networks->list = realloc(networks->list, networks->size * sizeof(wifi_network_t));
    } else {
        networks->list = malloc(networks->size * sizeof(wifi_network_t));
    }
    if (networks->list == NULL) {
        ESP_LOGE(TAG, "networks list memory allocation failed");
        return 1;
    }
    networks->list[networks->size - 1] = network;
    return 0;
}

esp_err_t networks_wifi(wifi_networks_t* networks) {
    ESP_LOGI(TAG, "networks wifi");
    esp_err_t err = ESP_OK;

    wifi_scan_config_t scan_cfg = {};
    err = esp_wifi_scan_start(&scan_cfg, true);
    if (err == ESP_ERR_WIFI_TIMEOUT) {
        ESP_LOGE(TAG, "timeout error");
        return ESP_ERR_WIFI_TIMEOUT;
    } else if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s", esp_err_to_name(err));
        return 0;
    }

    esp_wifi_scan_stop();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s", esp_err_to_name(err));
        return 0;
    }

    uint16_t ap_num;
    err = esp_wifi_scan_get_ap_num(&ap_num);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s", esp_err_to_name(err));
        return 0;
    }

    ESP_LOGI(TAG, "Found networks: %d", (int)ap_num);

    if (ap_num > 0 && MAX_NETWORKS_SIZE) {
        wifi_ap_record_t records[MAX_NETWORKS_SIZE];
        err = esp_wifi_scan_get_ap_records(&ap_num, records);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "%s", esp_err_to_name(err));
            return 0;
        }

        for (uint16_t i = 0; i < ap_num; i++) {
            wifi_network_t network;
            network.ssid = (char*)malloc(strlen((char*)records[i].ssid) + 1);
            if (network.ssid == NULL) {
                ESP_LOGE(TAG, "network.ssid memory allocation failed");
                break;
            }
            sprintf(network.ssid, "%s", (char*)records[i].ssid);
            networks_push(networks, network);
        }

        if (ap_num <= 0) {
            ESP_LOGE(TAG, "not found");
            return 0;
        }
    }

    err = esp_wifi_clear_ap_list();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s", esp_err_to_name(err));
        return 0;
    }

    return err;
}

esp_err_t networks_free(wifi_networks_t* networks) {
    ESP_LOGI(TAG, "networks free");
    esp_err_t err = ESP_OK;

    for (int i = 0; i < networks->size; i++) {
        free(networks->list[i].ssid);
    }
    free(networks->list);

    return err;
}

esp_err_t disconnect_wifi() {
    ESP_LOGI(TAG, "disconnect wifi");
    esp_err_t err = esp_wifi_disconnect();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s", esp_err_to_name(err));
    }
    return err;
}

esp_err_t join_wifi(wifi_network_t* network) {
    ESP_LOGI(TAG, "join_wifi ssid: %s, password: %s", network->ssid, network->password);
    esp_err_t err = ESP_OK;

    disconnect_wifi();

    wifi_status = CONNECTING;
    wifi_connection_retries = 0;

    wifi_config_t wifi_config = {
        .sta = {}
    };

    strlcpy((char*)wifi_config.sta.ssid, network->ssid, strlen(network->ssid) + 1);
    strlcpy((char*)wifi_config.sta.password, network->password, strlen(network->password) + 1);

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    // int esp_wifi_connect_attempts = 0;
    // do {
    //     esp_wifi_connect_attempts++;
    //     wifi_status = DISCONNECTED;

    //     err = esp_wifi_connect();

    //     if (err == ESP_OK) {
    //         ESP_LOGI(TAG, "call connecting success");
    //         break;
    //     } if (err == ESP_ERR_WIFI_CONN) {
    //         ESP_LOGE(TAG, "esp err connection error, wait 1 second...");
    //         vTaskDelay(pdMS_TO_TICKS(1000));
    //     } else if (err != ESP_ERR_WIFI_CONN) {
    //         ESP_LOGE(TAG, "esp err other error");
    //         break;
    //     }
    // } while (esp_wifi_connect_attempts < MAX_WIFI_CONNECTION_RETRIES);

    err = esp_wifi_connect();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "%s", esp_err_to_name(err));
        wifi_status = DISCONNECTED;
        if (err == ESP_ERR_WIFI_CONN) {
            return ESP_ERR_WIFI_CONN;
        } else {
            return err;
        }
    }

    ESP_LOGI(TAG, "join state %s", CONNECTION_STATE_NAMES[wifi_status]);
    while(wifi_status != CONNECTED && wifi_status != DISCONNECTED) {
        ESP_LOGI(TAG, "Connecting status...");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    return err;
}

esp_err_t netstat_wifi(wifi_netstat_t* netstat) {
    ESP_LOGI(TAG, "netstat_wifi");
    esp_err_t err = ESP_OK;

    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);

    wifi_config_t wifi_config;
    esp_wifi_get_config(ESP_IF_WIFI_STA, &wifi_config);

    netstat->mode = mode;
    netstat->network = "";
    if (strcmp((char*)wifi_config.sta.ssid, "") != 0) {
        netstat->network = (char*)malloc(strlen((char*)wifi_config.sta.ssid) + 1);
        strlcpy((char*)netstat->network, (char*)wifi_config.sta.ssid, strlen((char*)wifi_config.sta.ssid) + 1);
    }
    netstat->connected = (wifi_status == CONNECTED);
    netstat->ip = ip;
    netstat->status = wifi_status;

    return err;
}