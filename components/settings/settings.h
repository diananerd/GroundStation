#pragma once
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef cJSON* settings_handle_t;

esp_err_t settings_create(settings_handle_t* settings);
esp_err_t settings_get_str(settings_handle_t* settings, char* key, char** read_value);
esp_err_t settings_set_str(settings_handle_t* settings, char* key, char* value);
esp_err_t settings_get_int(settings_handle_t* settings, char* key, int* read_value);
esp_err_t settings_set_int(settings_handle_t* settings, char* key, int value);
void settings_free(settings_handle_t* settings);

#ifdef __cplusplus
}
#endif
