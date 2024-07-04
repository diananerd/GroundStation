idf_component_register(
    SRCS "api_console.c"
    INCLUDE_DIRS "."
    REQUIRES nvs_flash json console api
)