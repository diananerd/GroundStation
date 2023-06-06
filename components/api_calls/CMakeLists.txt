idf_component_register(
    SRCS "api_calls.c"
    INCLUDE_DIRS .
    REQUIRES esp_http_client
    EMBED_TXTFILES google_com_root_cert.pem amazonaws_com_root_cert.pem platzi_com_root_cert.pem
)