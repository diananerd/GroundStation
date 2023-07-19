#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "cmd_board.h"

#define STORAGE_BOARD_NAMESPACE "board"

static const char* TAG = "SetupBoard";

nvs_handle_t board_handle;

typedef struct {
    char *name;
    int32_t type;
    int32_t aADDR;
    int32_t oSDA;
    int32_t oSCL;
    int32_t oRST;
    int32_t pBut;
    int32_t led;
    int32_t lNSS;
    int32_t lDIO0;
    int32_t lDIO1;
    int32_t lBUSSY;
    int32_t lRST;
    int32_t lMISO;
    int32_t lMOSI;
    int32_t lSCK;
} board_settings_t;

// Default values for TTGO LoRa v2 433 MHz board
static board_settings_t board_settings =  {
    .name = "TTGO_LoRA_v2_433MHz",
    .type = 0,
    .aADDR = 60,
    .oSDA = 21,
    .oSCL = 22,
    .oRST = 16,
    .pBut = 0,
    .led = 22,
    .lNSS = 18,
    .lDIO0 = 26,
    .lDIO1 = 33,
    .lBUSSY = 0,
    .lRST = 14,
    .lMISO = 19,
    .lMOSI = 27,
    .lSCK = 5
};

static struct {
    struct arg_end *end;
} get_board_settings_args;

static struct {
    struct arg_str *name;
    struct arg_int *type;
    struct arg_int *aADDR;
    struct arg_int *oSDA;
    struct arg_int *oSCL;
    struct arg_int *oRST;
    struct arg_int *pBut;
    struct arg_int *led;
    struct arg_int *lNSS;
    struct arg_int *lDIO0;
    struct arg_int *lDIO1;
    struct arg_int *lBUSSY;
    struct arg_int *lRST;
    struct arg_int *lMISO;
    struct arg_int *lMOSI;
    struct arg_int *lSCK;
    struct arg_end *end;
} set_board_settings_args;

esp_err_t init_board_settings() {
    ESP_LOGI(TAG, "Init NVS board settings");
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
    return err;
}

static int get_board_settings_handler(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &get_board_settings_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, get_board_settings_args.end, argv[0]);
        return 1;
    }

    esp_err_t err = nvs_open(STORAGE_BOARD_NAMESPACE, NVS_READONLY, &board_handle);
    if (err != ESP_OK) return err;

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error on read NVS board settings");
        return 0;
    }

    size_t required_size;
    nvs_get_str(board_handle, "bs_name", NULL, &required_size);
    board_settings.name = malloc(required_size);

    if (nvs_get_str(board_handle, "bs_name", board_settings.name, &required_size) == ESP_OK) {
        ESP_LOGI(TAG, "Board name = %s", board_settings.name);
    } else {
        ESP_LOGE(TAG, "Board name not found");
    }

    if (nvs_get_i32(board_handle, "bs_type", &board_settings.type) == ESP_OK) {
        if (board_settings.type == 0) {
            ESP_LOGI(TAG, "LoRa module frecuency is 433 MHz");
        } else if (board_settings.type == 1) {
            ESP_LOGI(TAG, "LoRa module frecuency is 915 MHz");
        } else {
            ESP_LOGE(TAG, "LoRa module frecuency invalid value = %li", board_settings.type);
        }
    }
    if (nvs_get_i32(board_handle, "bs_aADDR", &board_settings.aADDR) == ESP_OK) {
        ESP_LOGI(TAG, "OLED address in decimal value = %li", board_settings.aADDR);
    }
    if (nvs_get_i32(board_handle, "bs_oSDA", &board_settings.oSDA) == ESP_OK) {
        ESP_LOGI(TAG, "OLED SDA pin = %li", board_settings.oSDA);
    }
    if (nvs_get_i32(board_handle, "bs_oSCL", &board_settings.oSCL) == ESP_OK) {
        ESP_LOGI(TAG, "OLED SCL pin = %li", board_settings.oSCL);
    }
    if (nvs_get_i32(board_handle, "bs_oRST", &board_settings.oRST) == ESP_OK) {
        ESP_LOGI(TAG, "OLED RST pin = %li", board_settings.oRST);
    }
    if (nvs_get_i32(board_handle, "bs_pBut", &board_settings.pBut) == ESP_OK) {
        ESP_LOGI(TAG, "User button pin = %li", board_settings.pBut);
    }
    if (nvs_get_i32(board_handle, "bs_led", &board_settings.led) == ESP_OK) {
        ESP_LOGI(TAG, "Board LED pin = %li", board_settings.led);
    }
    if (nvs_get_i32(board_handle, "bs_lNSS", &board_settings.lNSS) == ESP_OK) {
        ESP_LOGI(TAG, "LoRa NSS/CS pin = %li", board_settings.lNSS);
    }
    if (nvs_get_i32(board_handle, "bs_lDIO0", &board_settings.lDIO0) == ESP_OK) {
        ESP_LOGI(TAG, "LoRa DIO0 pin = %li", board_settings.lDIO0);
    }
    if (nvs_get_i32(board_handle, "bs_lDIO1", &board_settings.lDIO1) == ESP_OK) {
        ESP_LOGI(TAG, "LoRa DIO1 pin = %li", board_settings.lDIO1);
    }
    if (nvs_get_i32(board_handle, "bs_lBUSSY", &board_settings.lBUSSY) == ESP_OK) {
        ESP_LOGI(TAG, "LoRa BUSSY pin = %li", board_settings.lBUSSY);
    }
    if (nvs_get_i32(board_handle, "bs_lRST", &board_settings.lRST) == ESP_OK) {
        ESP_LOGI(TAG, "LoRa RST pin = %li", board_settings.lRST);
    }
    if (nvs_get_i32(board_handle, "bs_lMISO", &board_settings.lMISO) == ESP_OK) {
        ESP_LOGI(TAG, "LoRa MISO pin = %li", board_settings.lMISO);
    }
    if (nvs_get_i32(board_handle, "bs_lMOSI", &board_settings.lMOSI) == ESP_OK) {
        ESP_LOGI(TAG, "LoRa MOSI pin = %li", board_settings.lMOSI);
    }
    if (nvs_get_i32(board_handle, "bs_lSCK", &board_settings.lSCK) == ESP_OK) {
        ESP_LOGI(TAG, "LoRa SCK pin = %li", board_settings.lSCK);
    }

    nvs_close(board_handle);
    ESP_LOGI(TAG, "read board settings end");
    return 0;
}

static int set_board_settings_handler(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &set_board_settings_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, set_board_settings_args.end, argv[0]);
        return 1;
    }

    esp_err_t err = nvs_open(STORAGE_BOARD_NAMESPACE, NVS_READWRITE, &board_handle);
    if (err != ESP_OK) return err;

    if (set_board_settings_args.name->count > 0) {
        ESP_LOGI(TAG, "Board name = %s", set_board_settings_args.name->sval[0]);
        err = nvs_set_str(board_handle, "bs_name", set_board_settings_args.name->sval[0]);
        ESP_ERROR_CHECK( err );
    }
    if (set_board_settings_args.type->count > 0) {
        if (set_board_settings_args.type->ival[0] == 0) {
            ESP_LOGI(TAG, "LoRa module frecuency is 433 MHz");
        } else if (set_board_settings_args.type->ival[0] == 1) {
            ESP_LOGI(TAG, "LoRa module frecuency is 915 MHz");
        } else {
            ESP_LOGE(TAG, "LoRa module frecuency invalid value = %i", set_board_settings_args.type->ival[0]);
        }
        err = nvs_set_i32(board_handle, "bs_type", set_board_settings_args.type->ival[0]);
        ESP_ERROR_CHECK( err );
    }
    if (set_board_settings_args.aADDR->count > 0) {
        ESP_LOGI(TAG, "OLED address in decimal value = %i", set_board_settings_args.aADDR->ival[0]);
        err = nvs_set_i32(board_handle, "bs_aADDR", set_board_settings_args.aADDR->ival[0]);
        ESP_ERROR_CHECK( err );
    }
    if (set_board_settings_args.oSDA->count > 0) {
        ESP_LOGI(TAG, "OLED SDA pin = %i", set_board_settings_args.oSDA->ival[0]);
        err = nvs_set_i32(board_handle, "bs_oSDA", set_board_settings_args.oSDA->ival[0]);
        ESP_ERROR_CHECK( err );
    }
    if (set_board_settings_args.oSCL->count > 0) {
        ESP_LOGI(TAG, "OLED SCL pin = %i", set_board_settings_args.oSCL->ival[0]);
        err = nvs_set_i32(board_handle, "bs_oSCL", set_board_settings_args.oSCL->ival[0]);
        ESP_ERROR_CHECK( err );
    }
    if (set_board_settings_args.oRST->count > 0) {
        ESP_LOGI(TAG, "OLED RST pin = %i", set_board_settings_args.oRST->ival[0]);
        err = nvs_set_i32(board_handle, "bs_oRST", set_board_settings_args.oRST->ival[0]);
        ESP_ERROR_CHECK( err );
    }
    if (set_board_settings_args.pBut->count > 0) {
        ESP_LOGI(TAG, "User button pin = %i", set_board_settings_args.pBut->ival[0]);
        err = nvs_set_i32(board_handle, "bs_pBut", set_board_settings_args.pBut->ival[0]);
        ESP_ERROR_CHECK( err );
    }
    if (set_board_settings_args.led->count > 0) {
        ESP_LOGI(TAG, "Board LED pin = %i", set_board_settings_args.led->ival[0]);
        err = nvs_set_i32(board_handle, "bs_led", set_board_settings_args.led->ival[0]);
        ESP_ERROR_CHECK( err );
    }
    if (set_board_settings_args.lNSS->count > 0) {
        ESP_LOGI(TAG, "LoRa NSS/CS pin = %i", set_board_settings_args.lNSS->ival[0]);
        err = nvs_set_i32(board_handle, "bs_lNSS", set_board_settings_args.lNSS->ival[0]);
        ESP_ERROR_CHECK( err );
    }
    if (set_board_settings_args.lDIO0->count > 0) {
        ESP_LOGI(TAG, "LoRa DIO0 pin = %i", set_board_settings_args.lDIO0->ival[0]);
        err = nvs_set_i32(board_handle, "bs_lDIO0", set_board_settings_args.lDIO0->ival[0]);
        ESP_ERROR_CHECK( err );
    }
    if (set_board_settings_args.lDIO1->count > 0) {
        ESP_LOGI(TAG, "LoRa DIO1 pin = %i", set_board_settings_args.lDIO1->ival[0]);
        err = nvs_set_i32(board_handle, "bs_lDIO1", set_board_settings_args.lDIO1->ival[0]);
        ESP_ERROR_CHECK( err );
    }
    if (set_board_settings_args.lBUSSY->count > 0) {
        ESP_LOGI(TAG, "LoRa BUSSY pin = %i", set_board_settings_args.lBUSSY->ival[0]);
        err = nvs_set_i32(board_handle, "bs_lBUSSY", set_board_settings_args.lBUSSY->ival[0]);
        ESP_ERROR_CHECK( err );
    }
    if (set_board_settings_args.lRST->count > 0) {
        ESP_LOGI(TAG, "LoRa RST pin = %i", set_board_settings_args.lRST->ival[0]);
        err = nvs_set_i32(board_handle, "bs_lRST", set_board_settings_args.lRST->ival[0]);
        ESP_ERROR_CHECK( err );
    }
    if (set_board_settings_args.lMISO->count > 0) {
        ESP_LOGI(TAG, "LoRa MISO pin = %i", set_board_settings_args.lMISO->ival[0]);
        err = nvs_set_i32(board_handle, "bs_lMISO", set_board_settings_args.lMISO->ival[0]);
        ESP_ERROR_CHECK( err );
    }
    if (set_board_settings_args.lMOSI->count > 0) {
        ESP_LOGI(TAG, "LoRa MOSI pin = %i", set_board_settings_args.lMOSI->ival[0]);
        err = nvs_set_i32(board_handle, "bs_lMOSI", set_board_settings_args.lMOSI->ival[0]);
        ESP_ERROR_CHECK( err );
    }
    if (set_board_settings_args.lSCK->count > 0) {
        ESP_LOGI(TAG, "LoRa SCK pin = %i", set_board_settings_args.lSCK->ival[0]);
        err = nvs_set_i32(board_handle, "bs_lSCK", set_board_settings_args.lSCK->ival[0]);
        ESP_ERROR_CHECK( err );
    }

    err = nvs_commit(board_handle);
    ESP_ERROR_CHECK( err );
    nvs_close(board_handle);

    ESP_LOGI(TAG, "setup board end");
    return 0;
}

void register_board(void) {
    get_board_settings_args.end = arg_end(0);

    set_board_settings_args.name = arg_str1(NULL, "name", "<n>", "Board name without spaces (use underscore), please use: <brand> + <model> + <version> + <frequency> or if it is a github project: <user> + <repo> + <version> + <frequency>");
    set_board_settings_args.type = arg_int0(NULL, "type", "<type>", "Board radio type (0 = 433 MHz, 1 = 915 MHz)");
    set_board_settings_args.aADDR = arg_int0(NULL, "aADDR", "<aADDR>", "OLED I2C Address (in decimal format)");
    set_board_settings_args.oSDA = arg_int0(NULL, "oSDA", "<oSDA>", "OLED SDA pin");
    set_board_settings_args.oSCL = arg_int0(NULL, "oSCL", "<oSCL>", "OLED SCL pin");
    set_board_settings_args.oRST = arg_int0(NULL, "oRST", "<oRST>", "OLED RST pin");
    set_board_settings_args.pBut = arg_int0(NULL, "pBut", "<pBut>", "GPIO user button");
    set_board_settings_args.led = arg_int0(NULL, "led", "<led>", "GPIO board led");
    set_board_settings_args.lNSS = arg_int0(NULL, "lNSS", "<lNSS>", "LoRa NSS pin");
    set_board_settings_args.lDIO0 = arg_int0(NULL, "lDIO0", "<lDIO0>", "LoRa DIO0 pin");
    set_board_settings_args.lDIO1 = arg_int0(NULL, "lDIO1", "<lDIO1>", "LoRa DIO1 pin");
    set_board_settings_args.lBUSSY = arg_int0(NULL, "lBUSSY", "<lBUSSY>", "LoRa BUSSY pin");
    set_board_settings_args.lRST = arg_int0(NULL, "lRST", "<lRST>", "LoRa RST pin");
    set_board_settings_args.lMISO = arg_int0(NULL, "lMISO", "<lMISO>", "LoRa MISO pin");
    set_board_settings_args.lMOSI = arg_int0(NULL, "lMOSI", "<lMOSI>", "LoRa MOSI pin");
    set_board_settings_args.lSCK = arg_int0(NULL, "lSCK", "<lSCK>", "LoRa SCK pin");
    set_board_settings_args.end = arg_end(20);

    const esp_console_cmd_t get_board_settings_cmd = {
        .command = "get-board",
        .help = "Read board pinout",
        .hint = NULL,
        .func = &get_board_settings_handler,
        .argtable = &get_board_settings_args
    };

    const esp_console_cmd_t set_board_settings_cmd = {
        .command = "board",
        .help = "Setup board pinout",
        .hint = NULL,
        .func = &set_board_settings_handler,
        .argtable = &set_board_settings_args
    };

    init_board_settings();

    ESP_ERROR_CHECK(esp_console_cmd_register(&get_board_settings_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&set_board_settings_cmd));

    // Oficial supported boards
    // TTGO LoRa 433 MHz v2
    // board --name="TTGO_LoRA_v2_433MHz" --type=0 --aADDR=60 --oSDA=21 --oSCL=22 --oRST=16 --pBut=0 --led=22 --lNSS=18 --lDIO0=26 --lDIO1=33 --lBUSSY=0 --lRST=14 --lMISO=19 --lMOSI=27 --lSCK=5
    // Heltec LoRa 433 MHz v2
    // board --name="Heltec_LoRa_v2_433MHz" --type=0 --aADDR=60 --oSDA=4 --oSCL=15 --oRST=16 --pBut=0 --led=25 --lNSS=18 --lDIO0=26 --lDIO1=12 --lBUSSY=0 --lRST=14 --lMISO=19 --lMOSI=27 --lSCK=5
}