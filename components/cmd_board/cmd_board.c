#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "cmd_board.h"

static const char* TAG = "SetupBoard";

static struct {
    struct arg_str *name;
    struct arg_int *aADDR;
    struct arg_int *oSDA;
    struct arg_int *oSCL;
    struct arg_int *oRST;
    struct arg_int *pBut;
    struct arg_int *led;
    struct arg_int *radio;
    struct arg_int *lNSS;
    struct arg_int *lDIO0;
    struct arg_int *lDIO1;
    struct arg_int *lBUSSY;
    struct arg_int *lRST;
    struct arg_int *lMISO;
    struct arg_int *lMOSI;
    struct arg_int *lSCK;
    struct arg_str *lTCXOV;
    struct arg_end *end;
} setup_board_args;

static int setup_board(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &setup_board_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, setup_board_args.end, argv[0]);
        return 1;
    }
    ESP_LOGI(TAG, "board name = %s", setup_board_args.name->sval[0]);
    ESP_LOGI(TAG, "setup board end");
    return 0;
}

void register_board(void) {
    setup_board_args.name = arg_str1(NULL, "name", "<n>", "Board name");
    setup_board_args.aADDR = arg_int0(NULL, "aADDR", "<aADDR>", "OLED I2C Address (in decimal format)");
    setup_board_args.oSDA = arg_int0(NULL, "oSDA", "<oSDA>", "OLED SDA pin");
    setup_board_args.oSCL = arg_int0(NULL, "oSCL", "<oSCL>", "OLED SCL pin");
    setup_board_args.oRST = arg_int0(NULL, "oRST", "<oRST>", "OLED RST pin");
    setup_board_args.pBut = arg_int0(NULL, "pBut", "<pBut>", "GPIO user button");
    setup_board_args.led = arg_int0(NULL, "led", "<led>", "GPIO board led");
    setup_board_args.radio = arg_int0(NULL, "radio", "<radio>", "Type of radio (0 = sx166x, 1 = sx127x)");
    setup_board_args.lNSS = arg_int0(NULL, "lNSS", "<lNSS>", "LoRa NSS pin");
    setup_board_args.lDIO0 = arg_int0(NULL, "lDIO0", "<lDIO0>", "LoRa DIO0 pin");
    setup_board_args.lDIO1 = arg_int0(NULL, "lDIO1", "<lDIO1>", "LoRa DIO1 pin");
    setup_board_args.lBUSSY = arg_int0(NULL, "lBUSSY", "<lBUSSY>", "LoRa BUSSY pin");
    setup_board_args.lRST = arg_int0(NULL, "lRST", "<lRST>", "LoRa RST pin");
    setup_board_args.lMISO = arg_int0(NULL, "lMISO", "<lMISO>", "LoRa MISO pin");
    setup_board_args.lMOSI = arg_int0(NULL, "lMOSI", "<lMOSI>", "LoRa MOSI pin");
    setup_board_args.lSCK = arg_int0(NULL, "lSCK", "<lSCK>", "LoRa SCK pin");
    setup_board_args.lTCXOV = arg_str0(NULL, "lTCXOV", "<lTCXOV>", "LoRa TCXO voltage (float value, only for sx126x modules)");
    setup_board_args.end = arg_end(2);

    const esp_console_cmd_t setup_board_cmd = {
        .command = "board",
        .help = "Setup board pinout",
        .hint = NULL,
        .func = &setup_board,
        .argtable = &setup_board_args
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&setup_board_cmd));
}