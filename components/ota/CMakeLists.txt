idf_component_register(
    SRCS "ota.c"
    INCLUDE_DIRS "."
    REQUIRES esp_http_client esp_https_ota json
)