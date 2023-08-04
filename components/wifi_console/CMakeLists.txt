idf_component_register(
    SRCS "wifi_console.c"
    INCLUDE_DIRS "."
    REQUIRES nvs_flash esp_wifi console json wifi
)