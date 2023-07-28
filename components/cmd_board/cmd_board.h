#pragma once

#ifdef __cplusplus
extern "C" {
#endif

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

// Register WiFi functions
void initialize_board(void);
void register_board(void);
esp_err_t get_board_settings(board_settings_t *board_settings);

#ifdef __cplusplus
}
#endif
