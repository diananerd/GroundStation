idf_component_register(
    SRCS "api_calls.c"
    INCLUDE_DIRS .
    REQUIRES esp_http_client
    EMBED_TXTFILES platzi_com_root_cert.pem
)