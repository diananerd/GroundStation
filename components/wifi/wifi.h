#pragma once
#include "esp_wifi.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_NETWORKS_SIZE 50

typedef enum CONNECTION_STATES {
    CONNECTED,
    DISCONNECTED,
    CONNECTING
} wifi_status_t;

static const char * const CONNECTION_STATE_NAMES[] = {
    [CONNECTED] = "connected",
    [DISCONNECTED] = "disconnected",
    [CONNECTING] = "connecting"
};

typedef struct {
    wifi_mode_t mode;
    char* network;
    int connected; // 0 = disconnected, 1 = connected
    wifi_status_t status;
    char* ip;
} wifi_netstat_t;

typedef struct {
    char* ssid;
    char* password;
} wifi_network_t;

typedef struct {
    int size;
    wifi_network_t* list;
} wifi_networks_t;

esp_err_t initialize_wifi();
esp_err_t networks_wifi(wifi_networks_t* networks);
esp_err_t networks_free(wifi_networks_t* networks);
esp_err_t join_wifi(wifi_network_t* network);
esp_err_t netstat_wifi(wifi_netstat_t* netstat);

#ifdef __cplusplus
}
#endif