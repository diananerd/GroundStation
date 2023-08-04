idf_component_register(
    SRCS "settings_console.c"
    INCLUDE_DIRS "."
    REQUIRES json nvs_flash console settings
)