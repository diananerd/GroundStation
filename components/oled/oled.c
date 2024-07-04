#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "driver/i2c.h"
#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "oled.h"
#include "settings.h"

static const char* TAG = "OLED";
// #define LOG_LOCAL_LEVEL ESP_LOG_WARN

#define I2C_HOST  0

#define LCD_PIXEL_CLOCK_HZ    (400 * 1000)
#define I2C_HW_ADDR           0x3C

#define LCD_H_RES              128
#define LCD_V_RES              64

#define LCD_CMD_BITS           8
#define LCD_PARAM_BITS         8

lv_disp_t *disp;

/* The LVGL port component calls esp_lcd_panel_draw_bitmap API for send data to the screen. There must be called
lvgl_port_flush_ready(disp) after each transaction to display. The best way is to use on_color_trans_done
callback from esp_lcd IO config structure. In IDF 5.1 and higher, it is solved inside LVGL port component. */
static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx) {
    lv_disp_t * disp = (lv_disp_t *)user_ctx;
    lvgl_port_flush_ready(disp);
    return false;
}

esp_err_t initialize_oled() {
    ESP_LOGI(TAG, "Read OLED settings");
    settings_handle_t settings_handle;
    settings_create(&settings_handle);

    setting_t oled_sda = {
        .key = "oled_sda"
    };
    esp_err_t err = settings_get(&settings_handle, &oled_sda);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) reading oled_sda setting", esp_err_to_name(err));
        return err;
    }
    setting_t oled_scl = {
        .key = "oled_scl"
    };
    err = settings_get(&settings_handle, &oled_scl);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) reading oled_sda setting", esp_err_to_name(err));
        return err;
    }
    setting_t oled_rst = {
        .key = "oled_rst"
    };
    err = settings_get(&settings_handle, &oled_rst);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) reading oled_sda setting", esp_err_to_name(err));
        return err;
    }

    if (oled_sda.valueint == oled_scl.valueint) {
        ESP_LOGE(TAG, "Error SDA and SCL pins cannot be the same");
        return ESP_OK;
    }

    if (oled_sda.valueint <= 0) {
        ESP_LOGE(TAG, "Invalid SDA pin");
        return ESP_OK;
    }

    if (oled_scl.valueint <= 0) {
        ESP_LOGE(TAG, "Invalid SCL pin");
        return ESP_OK;
    }

    if (oled_rst.valueint < 0) {
        ESP_LOGE(TAG, "Invalid RST pin");
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Initialize I2C bus");
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = oled_sda.valueint,
        .scl_io_num = oled_scl.valueint,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = LCD_PIXEL_CLOCK_HZ,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_HOST, &i2c_conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_HOST, I2C_MODE_MASTER, 0, 0, 0));

    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = I2C_HW_ADDR,
        .control_phase_bytes = 1,               // According to SSD1306 datasheet
        .lcd_cmd_bits = LCD_CMD_BITS,   // According to SSD1306 datasheet
        .lcd_param_bits = LCD_CMD_BITS, // According to SSD1306 datasheet
        .dc_bit_offset = 6,                     // According to SSD1306 datasheet
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)I2C_HOST, &io_config, &io_handle));

    ESP_LOGI(TAG, "Install SSD1306 panel driver");
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .bits_per_pixel = 1,
        .reset_gpio_num = oled_rst.valueint,
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle));

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    ESP_LOGI(TAG, "Initialize LVGL");
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    lvgl_port_init(&lvgl_cfg);

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = LCD_H_RES * LCD_V_RES,
        .double_buffer = true,
        .hres = LCD_H_RES,
        .vres = LCD_V_RES,
        .monochrome = true,
        .rotation = {
            .swap_xy = false,
            .mirror_x = true,
            .mirror_y = true,
        }
    };
    disp = lvgl_port_add_disp(&disp_cfg);
    /* Register done callback for IO */
    const esp_lcd_panel_io_callbacks_t cbs = {
        .on_color_trans_done = notify_lvgl_flush_ready,
    };
    esp_lcd_panel_io_register_event_callbacks(io_handle, &cbs, disp);

    /* Rotation of the screen */
    lv_disp_set_rotation(disp, LV_DISP_ROT_NONE);

    return ESP_OK;
}

void example_lvgl_demo_ui(lv_disp_t *disp) {
    lv_obj_t *scr = lv_disp_get_scr_act(disp);
    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR); /* Circular scroll */
    lv_label_set_text(label, "Hello Espressif, Hello LVGL.");
    /* Size of the screen (if you use rotation 90 or 270, please set disp->driver->ver_res) */
    lv_obj_set_width(label, disp->driver->hor_res);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);
}

esp_err_t demo_oled() {
    ESP_LOGI(TAG, "Display LVGL Scroll Text");
    example_lvgl_demo_ui(disp);
    return ESP_OK;
}