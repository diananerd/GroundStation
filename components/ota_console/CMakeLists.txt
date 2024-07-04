idf_component_register(
    SRCS "ota_console.c"
    INCLUDE_DIRS "."
    REQUIRES esp_http_client esp_https_ota json console ota
)