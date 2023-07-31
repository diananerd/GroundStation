#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "cJSON.h"
#include "settings.h"

static const char* TAG = "BoardSettings";
#define LOG_LOCAL_LEVEL ESP_LOG_WARN

esp_err_t settings_create(settings_handle_t* settings) {
    *settings = cJSON_CreateObject();
    return ESP_OK;
}

esp_err_t read_settings(settings_handle_t* settings) {
    ESP_LOGI(TAG, "Read board settings from namespace=%s", CONFIG_STORAGE_NAMESPACE);
    esp_err_t err = ESP_OK;
    nvs_handle_t settings_handle;
    err = nvs_open(CONFIG_STORAGE_NAMESPACE, NVS_READWRITE, &settings_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS fails to open namespace=\"%s\" with error: %s", CONFIG_STORAGE_NAMESPACE, esp_err_to_name(err));
        return err;
    }
    ESP_LOGI(TAG, "NVS open namespace=\"%s\"", CONFIG_STORAGE_NAMESPACE);

    size_t settings_string_size;
    err = nvs_get_str(settings_handle, "settings", NULL, &settings_string_size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error on get settings size from NVS");
        return err;
    }

    char *settings_string = (char*)malloc(settings_string_size);
    err = nvs_get_str(settings_handle, "settings", settings_string, &settings_string_size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error on read settings value from NVS");
        return err;
    }

    ESP_LOGI(TAG, "Readed settings_string\n%s", settings_string);
    ESP_LOGI(TAG, "Parsing settings_string to cJSON");
    *settings = cJSON_Parse(settings_string);

    free(settings_string);

    nvs_close(settings_handle);
    return err;
}

esp_err_t write_settings(settings_handle_t settings) {
    ESP_LOGI(TAG, "Write board settings from namespace=%s", CONFIG_STORAGE_NAMESPACE);
    esp_err_t err = ESP_OK;
    nvs_handle_t settings_handle;
    err = nvs_open(CONFIG_STORAGE_NAMESPACE, NVS_READWRITE, &settings_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "NVS fails to open namespace=\"%s\" with error: %s", CONFIG_STORAGE_NAMESPACE, esp_err_to_name(err));
        return err;
    }
    ESP_LOGI(TAG, "NVS open namespace=\"%s\"", CONFIG_STORAGE_NAMESPACE);

    char *settings_string = cJSON_Print(settings);
    ESP_LOGI(TAG, "Save settings as string\n%s", settings_string);

    err = nvs_set_str(settings_handle, "settings", settings_string);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error on write settings_string value to NVS");
    }

    err = nvs_commit(settings_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error on commit settings_string value to NVS");
    }

    cJSON_free(settings_string);

    nvs_close(settings_handle);
    return err;
}

esp_err_t settings_get_str(settings_handle_t* settings, char* key, char** read_value) {
    esp_err_t err = ESP_OK;
    err = read_settings(settings);
    if (!cJSON_GetObjectItem(*settings, key)) {
      ESP_LOGE(TAG, "%s value not found", key);
      return ESP_ERR_NOT_FOUND;
    }
    char* value = cJSON_GetObjectItem(*settings, key)->valuestring;
    ESP_LOGI(TAG, "key=%s value = %s", key, value);
    *read_value = (char*)malloc(strlen(value) + 1);
    strcpy(*read_value, value);
    ESP_LOGI(TAG, "settings_get_str key=%s value=%s", key, value);
    return err;
}

esp_err_t settings_set_str(settings_handle_t* settings, char* key, char* value) {
    ESP_LOGI(TAG, "settings_set_str key=%s value=%s", key, value);
    esp_err_t err = ESP_OK;
    if (cJSON_GetObjectItem(*settings, key)) {
      cJSON_DeleteItemFromObjectCaseSensitive(*settings, key);
    }
    cJSON_AddStringToObject(*settings, key, value);
    err = write_settings(*settings);
    return err;
}

esp_err_t settings_get_int(settings_handle_t* settings, char* key, int* read_value) {
    esp_err_t err = ESP_OK;
    err = read_settings(settings);
    if (!cJSON_GetObjectItem(*settings, key)) {
      ESP_LOGE(TAG, "%s value not found", key);
      return ESP_ERR_NOT_FOUND;
    }
    int value = cJSON_GetObjectItem(*settings, key)->valueint;
    ESP_LOGI(TAG, "key=%s value = %i", key, value);
    *read_value = value;
    ESP_LOGI(TAG, "settings_get_int key=%s value=%i", key, value);
    return err;
}

esp_err_t settings_set_int(settings_handle_t* settings, char* key, int value) {
    ESP_LOGI(TAG, "settings_set_int key=%s value=%i", key, value);
    esp_err_t err = ESP_OK;
    if (cJSON_GetObjectItem(*settings, key)) {
      cJSON_DeleteItemFromObjectCaseSensitive(*settings, key);
    }
    cJSON_AddNumberToObject(*settings, key, value);
    err = write_settings(*settings);
    return err;
}

void settings_free(settings_handle_t* settings) {
    cJSON_free(*settings);
}