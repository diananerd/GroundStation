idf_component_register(
    SRCS "settings.c"
    INCLUDE_DIRS "."
    REQUIRES json nvs_flash
)