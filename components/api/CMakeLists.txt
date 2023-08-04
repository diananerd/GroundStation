idf_component_register(
    SRCS "api.c"
    INCLUDE_DIRS "."
    REQUIRES nvs_flash json
)