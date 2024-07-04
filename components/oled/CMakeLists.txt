idf_component_register(
    SRCS "oled.c"
    INCLUDE_DIRS "."
    REQUIRES esp_timer esp_lcd driver lvgl esp_lvgl_port settings
)