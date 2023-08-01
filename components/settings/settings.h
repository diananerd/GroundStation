#pragma once
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef cJSON* settings_handle_t;

typedef enum SETTING_TYPES {
    STRING,
    NUMBER
} setting_type_t;

static const char * const SETTING_TYPE_NAMES[] = {
    [STRING] = "string",
    [NUMBER] = "number"
};

typedef struct {
    setting_type_t type;
    char* key;
    char* valuestring;
    int valueint;
} setting_t;

esp_err_t settings_create(settings_handle_t* settings);
esp_err_t settings_get(settings_handle_t* settings, setting_t* setting);
esp_err_t settings_set(settings_handle_t* settings, const setting_t* setting);
esp_err_t settings_raw_str(char** settings_string);
esp_err_t settings_list();
void settings_free(settings_handle_t* settings);

#ifdef __cplusplus
}
#endif
