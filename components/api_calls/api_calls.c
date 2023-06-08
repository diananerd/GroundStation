#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/Task.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "api_calls.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "cJSON.h"

#define SPACE_API_BASE_URL "https://api-sls.platzi.com/prod/space-api"
#define MAX_URL_SIZE 512
#define REQUEST_WAIT_TIME_INCREMENT_MS 2500

extern const char platzi_com_root_cert_pem_start[] asm("_binary_platzi_com_root_cert_pem_start");
extern const char platzi_com_root_cert_pem_end[]   asm("_binary_platzi_com_root_cert_pem_end");

static const char *TAG = "API_CALLS";

char* code;
char* verification_uri;
int interval;
int expires_in;

nvs_handle_t session;

bool get_tokens(char *access_token, char *refresh_token) {
    ESP_LOGI(TAG, "Load tokens from NVS");
    int32_t access_token_len = 0;
    int32_t refresh_token_len = 0;
    ESP_ERROR_CHECK(nvs_get_i32(session, "access_len", &access_token_len));
    ESP_ERROR_CHECK(nvs_get_i32(session, "refresh_len", &refresh_token_len));
    ESP_LOGI(TAG, "Access token len: %li", access_token_len);
    ESP_LOGI(TAG, "Refresh token len: %li", refresh_token_len);

    if (access_token_len) {
        ESP_LOGI(TAG, "Access token available, length %li", access_token_len);
        size_t access_token_size = access_token_len+1;
        access_token = (char*)malloc(access_token_size);
        ESP_ERROR_CHECK(nvs_get_str(session, "access_token", access_token, &access_token_size));
        access_token[access_token_len] = '\0';
        ESP_LOGI(TAG, "access token: %s", access_token);
    }
    if (refresh_token_len) {
        ESP_LOGI(TAG, "Refresh token available, length %li", refresh_token_len);
        size_t refresh_token_size = refresh_token_len+1;
        refresh_token = (char*)malloc(refresh_token_size);
        nvs_get_str(session, "refresh_token", refresh_token, &refresh_token_size);
        refresh_token[refresh_token_len] = '\0';
        ESP_LOGI(TAG, "refresh token: %s", refresh_token);
    }
    if (access_token_len || refresh_token_len) {
        ESP_LOGI(TAG, "NVS recovered session");
    } else {
        ESP_LOGI(TAG, "NVS session not found");
    }
    return 0;
}

bool save_token(char* token_name, char* token) {
    int32_t token_len = (int32_t)strlen(token);
    ESP_LOGI(TAG, "Save %s to nvs: %s, len: %li", token_name, token, token_len);
    if (strcmp(token_name, "access_token") == 0) {
        ESP_ERROR_CHECK(nvs_set_i32(session, "access_len", token_len));
        ESP_ERROR_CHECK(nvs_set_str(session, "access_token", token));
    } else if (strcmp(token_name, "refresh_token")  == 0) {
        ESP_ERROR_CHECK(nvs_set_i32(session, "refresh_len", token_len));
        ESP_ERROR_CHECK(nvs_set_str(session, "refresh_token", token));
    } else {
        ESP_LOGI(TAG, "Unknown token name: %s", token_name);
    }
    return 0;
}

bool clear_session() {
    ESP_LOGI(TAG, "Clear session in NVS");
    save_token("access_token", "");
    save_token("refresh_token", "");
    return 0;
}

void nvs_session_init() {
    ESP_LOGI(TAG, "NVS session init");
    nvs_flash_init();
    nvs_open("session", NVS_READWRITE, &session);
    char* access = {0};
    char* refresh = {0};
    esp_err_t err = get_tokens(access, refresh);
    if (err != ESP_OK) {
        ESP_LOGI(TAG, "Error on get tokens");
    }
}

esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    static char *output_buffer;  // Buffer to store response of http request from event handler
    static int output_len;       // Stores number of bytes read
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            /*
             *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
             *  However, event handler can also be used in case chunked encoding is used.
             */
            if (!esp_http_client_is_chunked_response(evt->client)) {
                // If user_data buffer is configured, copy the response into the buffer
                int copy_len = 0;
                if (evt->user_data) {
                    copy_len = MIN(evt->data_len, (DEFAULT_HTTP_BUF_SIZE - output_len));
                    if (copy_len) {
                        memcpy(evt->user_data + output_len, evt->data, copy_len);
                    }
                } else {
                    const int buffer_len = esp_http_client_get_content_length(evt->client);
                    if (output_buffer == NULL) {
                        output_buffer = (char *) malloc(buffer_len);
                        output_len = 0;
                        if (output_buffer == NULL) {
                            ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    copy_len = MIN(evt->data_len, (buffer_len - output_len));
                    if (copy_len) {
                        memcpy(output_buffer + output_len, evt->data, copy_len);
                    }
                }
                output_len += copy_len;
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            if (output_buffer != NULL) {
                // Response is accumulated in output_buffer. Uncomment the below line to print the accumulated response
                // ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
            if (output_buffer != NULL) {
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_REDIRECT:
            ESP_LOGD(TAG, "HTTP_EVENT_REDIRECT");
            esp_http_client_set_redirection(evt->client);
            break;
    }
    return ESP_OK;
}

bool get_url(const char *url, int timeout_ms) {
    printf("Get url: %s, timeout: %ims\n", url, timeout_ms);
    esp_http_client_config_t config = {
        .url = url,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .cert_pem = platzi_com_root_cert_pem_start,
        .event_handler = _http_event_handler,
        .timeout_ms = timeout_ms,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    int status_code = esp_http_client_get_status_code(client);
    uint64_t content_length = esp_http_client_get_content_length(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %"PRIu64, status_code, content_length);
    } else {
        ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
    return 0;
}

bool http_get(char* url, char* res) {
    ESP_LOGI(TAG, "HTTP GET %s buff_size: %i\n", url, DEFAULT_HTTP_BUF_SIZE);
    char local_response_buffer[DEFAULT_HTTP_BUF_SIZE] = {0};
    esp_http_client_config_t config = {
        .url = url,
        .transport_type = HTTP_TRANSPORT_OVER_SSL,
        .cert_pem = platzi_com_root_cert_pem_start,
        .user_data = local_response_buffer,
        .event_handler = _http_event_handler,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_perform(client);

    int status_code = esp_http_client_get_status_code(client);
    uint64_t content_length = esp_http_client_get_content_length(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %"PRIu64, status_code, content_length);
        ESP_LOG_BUFFER_HEX(TAG, local_response_buffer, strlen(local_response_buffer));
        ESP_LOGI(TAG, "Decoded: %s", (char *)local_response_buffer);
        strcpy(res, local_response_buffer);
    } else {
        ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
    return 0;
}

void get_token_task(void *pvParameter) {
    // Poll tokens
    ESP_LOGI(TAG, "Starting Get Token Task");
    ESP_LOGI(TAG, "Device code: %ss...", code);
    ESP_LOGI(TAG, "Polling every: %is...", interval);
    ESP_LOGI(TAG, "Expiration in: %ims...", expires_in);
    int request_wait_time_ms = interval * 1000;
    while(true) {
        while(true) {
            // Fetch tokens
            char url[MAX_URL_SIZE] = "https://api-sls.platzi.com/prod/space-api/auth/token?code=";
            strcat(url, code);
            ESP_LOGI(TAG, "Get token url: %s\n", url);

            char resp[DEFAULT_HTTP_BUF_SIZE] = {0};
            http_get(url, resp);
            printf("device_code: %s\n", resp);

            cJSON *json = cJSON_Parse(resp);
            if (cJSON_GetObjectItem(json, "message")) {
                char *message = cJSON_GetObjectItem(json, "message")->valuestring;
                ESP_LOGI(TAG, "message=%s", message);
            }
            if (cJSON_GetObjectItem(json, "error")) {
                ESP_LOGI(TAG, "Get tokens error, show error...");
                char *error = cJSON_GetObjectItem(json,"error")->valuestring;
                ESP_LOGI(TAG, "error=%s", error);
                if (strcmp(error, "validation_error") == 0) {
                    ESP_LOGI(TAG, "Validation error");
                    break;
                } else if (strcmp(error, "denied") == 0) {
                    ESP_LOGI(TAG, "Denied error");
                    break;
                } else if (strcmp(error, "expired") == 0) {
                    ESP_LOGI(TAG, "Expiration error");
                    break;
                } else if (strcmp(error, "slow_down") == 0) {
                    request_wait_time_ms += REQUEST_WAIT_TIME_INCREMENT_MS;
                    ESP_LOGI(TAG, "Slow down error, new time: %ims", request_wait_time_ms);
                } else if (strcmp(error, "authorization_pending") == 0) {
                    ESP_LOGI(TAG, "Authorization pending");
                } else if (strcmp(error, "") != 0) {
                    ESP_LOGI(TAG, "Unknown error: %s", error);
                    break;
                }
            } else {
                ESP_LOGI(TAG, "Get tokens success, save tokens...");
                char* access_token = {0};
                char* refresh_token = {0};

                if (cJSON_GetObjectItem(json, "access_token")) {
                    char *access = cJSON_GetObjectItem(json, "access_token")->valuestring;
                    access_token = (char *)malloc(strlen(access)+1);
                    strcpy(access_token, access);
                    ESP_LOGI(TAG, "access_token=%s", access_token);
                    save_token("access_token", access_token);
                }
                if (cJSON_GetObjectItem(json, "refresh_token")) {
                    char *refresh = cJSON_GetObjectItem(json, "refresh_token")->valuestring;
                    refresh_token = (char *)malloc(strlen(refresh)+1);
                    strcpy(refresh_token, refresh);
                    ESP_LOGI(TAG, "refresh_token=%s", refresh_token);
                    save_token("refresh_token", refresh_token);
                }
                ESP_LOGI(TAG, "Tokens saved, delete get token task");
                vTaskDelete(NULL);
            }

            cJSON_Delete(json);

            ESP_LOGI(TAG, "End of get task cycle, wait for next");
            vTaskDelay(request_wait_time_ms / portTICK_PERIOD_MS);
        }

        ESP_LOGI(TAG, "Delete get token task");
        vTaskDelete(NULL);
    }
}

bool sync_account() {
    // Get device code
    ESP_LOGI(TAG, "Get device code");
    char url[] = "https://api-sls.platzi.com/prod/space-api/auth/code";
    ESP_LOGI(TAG, "Sync account url: %s\n", url);
    char resp[DEFAULT_HTTP_BUF_SIZE] = {0};
    http_get(url, resp);
    printf("device_code: %s\n", resp);
    cJSON *json = cJSON_Parse(resp);
    if (cJSON_GetObjectItem(json, "code")) {
        char* c = cJSON_GetObjectItem(json, "code")->valuestring;
        code = (char *)malloc(strlen(c)+1);
		strcpy(code, c);
		ESP_LOGI(TAG, "code=%s", code);
	}
    if (cJSON_GetObjectItem(json, "verification_uri")) {
		char* v = cJSON_GetObjectItem(json, "verification_uri")->valuestring;
        verification_uri = (char *)malloc(strlen(v)+1);
		strcpy(verification_uri, v);
		ESP_LOGI(TAG, "verification_uri=%s", verification_uri);
	}
    if (cJSON_GetObjectItem(json, "interval")) {
		interval = cJSON_GetObjectItem(json, "interval")->valueint;
		ESP_LOGI(TAG, "interval=%i", interval);
	}
    if (cJSON_GetObjectItem(json, "expires_in")) {
		expires_in = cJSON_GetObjectItem(json, "expires_in")->valueint;
		ESP_LOGI(TAG, "expires_in=%i", expires_in);
	}
    cJSON_Delete(json);
    xTaskCreate(&get_token_task, "get_token_task", 1024 * 8, NULL, 5, NULL);
    return 0;
}