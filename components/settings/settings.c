#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "settings.h"

static const char* TAG = "BoardSettings";
#define LOG_LOCAL_LEVEL ESP_LOG_WARN

esp_err_t settings_create(settings_handle_t* settings) {
    *settings = cJSON_CreateObject();
    read_settings(settings);
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
    ESP_LOGI(TAG, "NVS Commit");

    cJSON_free(settings_string);

    nvs_close(settings_handle);
    return err;
}

esp_err_t settings_get(settings_handle_t* settings, setting_t* setting) {
    esp_err_t err = ESP_OK;
    err = read_settings(settings);
    cJSON* setting_json = cJSON_GetObjectItem(*settings, setting->key);
    if (!setting_json) {
      ESP_LOGE(TAG, "%s value not found", setting->key);
      return ESP_ERR_NOT_FOUND;
    }
    ESP_LOGI(TAG, "Reading key=%s", setting->key);
    if (cJSON_IsString(setting_json)) {
        setting->type = STRING;
        char* value = setting_json->valuestring;
        setting->valuestring = (char*)malloc(strlen(value) + 1);
        strcpy(setting->valuestring, value);
        ESP_LOGI(TAG, "settings_get as str key=%s value=%s", setting->key, setting->valuestring);
    } else if (cJSON_IsNumber(setting_json)) {
        setting->type = NUMBER;
        setting->valueint = setting_json->valueint;
        ESP_LOGI(TAG, "settings_get as int key=%s value=%i", setting->key, setting->valueint);
    }
    return err;
}

esp_err_t settings_set(settings_handle_t* settings, const setting_t* setting) {
    esp_err_t err = ESP_OK;
    if (cJSON_GetObjectItem(*settings, setting->key)) {
      cJSON_DeleteItemFromObjectCaseSensitive(*settings, setting->key);
    }
    if (setting->type == STRING) {
        cJSON_AddStringToObject(*settings, setting->key, setting->valuestring);
    } else if (setting->type == NUMBER) {
        cJSON_AddNumberToObject(*settings, setting->key, setting->valueint);
    }
    err = write_settings(*settings);
    return err;
}

esp_err_t settings_delete(settings_handle_t* settings, const setting_t* setting) {
    esp_err_t err = ESP_OK;
    if (cJSON_GetObjectItem(*settings, setting->key)) {
      cJSON_DeleteItemFromObjectCaseSensitive(*settings, setting->key);
    }
    err = write_settings(*settings);
    return err;
}

esp_err_t settings_raw_str(char** settings_string) {
    esp_err_t err = ESP_OK;
    settings_handle_t settings;
    err = read_settings(&settings);
    char* raw_str = cJSON_Print(settings);
    *settings_string = (char*)malloc(strlen(raw_str));
    strcpy(*settings_string, raw_str);
    return err;
}

esp_err_t settings_list() {
    esp_err_t err = ESP_OK;
    settings_handle_t settings;
    err = read_settings(&settings);
    cJSON* element = settings->child;
    setting_t setting = {};
    while (element) {
        setting.key = (char*)malloc(strlen(element->string) + 1);
        strcpy(setting.key, element->string);
        err = settings_get(&settings, &setting);
        if (setting.type == STRING) {
            printf("%s: %s\n", element->string, setting.valuestring);
        } else if (setting.type == NUMBER) {
            printf("%s: %i\n", element->string, setting.valueint);
        }
        element = element->next;
    }
    return err;
}

esp_err_t settings_list_json() {
    esp_err_t err = ESP_OK;
    char* raw_str;
    err = settings_raw_str(&raw_str);
    if (err == ESP_OK) {
        printf("%s\n", raw_str);
    }
    return err;
}

void settings_free(settings_handle_t* settings) {
    cJSON_free(*settings);
}