#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "api.h"
#include "api_console.h"

static const char* TAG = "API_CONSOLE";
// #define LOG_LOCAL_LEVEL ESP_LOG_WARN

static struct {
    struct arg_end *end;
} login_api_args;

static int login_api_exec(int argc, char **argv) {
    ESP_LOGI(TAG, "login_api_exec");
    int nerrors = arg_parse(argc, argv, (void **) &login_api_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, login_api_args.end, argv[0]);
        return 1;
    }
    return 0;
}

static struct {
    struct arg_end *end;
} logout_api_args;

static int logout_api_exec(int argc, char **argv) {
    ESP_LOGI(TAG, "logout_api_exec");
    int nerrors = arg_parse(argc, argv, (void **) &logout_api_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, logout_api_args.end, argv[0]);
        return 1;
    }
    return 0;
}

static struct {
    struct arg_end *end;
} sync_api_args;

static int sync_api_exec(int argc, char **argv) {
    ESP_LOGI(TAG, "sync_api_exec");
    int nerrors = arg_parse(argc, argv, (void **) &sync_api_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, sync_api_args.end, argv[0]);
        return 1;
    }
    return 0;
}

static struct {
    struct arg_end *end;
} account_api_args;

static int account_api_exec(int argc, char **argv) {
    ESP_LOGI(TAG, "account_api_exec");
    int nerrors = arg_parse(argc, argv, (void **) &account_api_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, account_api_args.end, argv[0]);
        return 1;
    }
    return 0;
}

static struct {
    struct arg_end *end;
} session_api_args;

static int session_api_exec(int argc, char **argv) {
    ESP_LOGI(TAG, "session_api_exec");
    int nerrors = arg_parse(argc, argv, (void **) &session_api_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, session_api_args.end, argv[0]);
        return 1;
    }
    return 0;
}

static struct {
    struct arg_end *end;
} code_api_args;

static int code_api_exec(int argc, char **argv) {
    ESP_LOGI(TAG, "code_api_exec");
    int nerrors = arg_parse(argc, argv, (void **) &code_api_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, code_api_args.end, argv[0]);
        return 1;
    }
    return 0;
}

static struct {
    struct arg_end *end;
} tokens_api_args;

static int tokens_api_exec(int argc, char **argv) {
    ESP_LOGI(TAG, "tokens_api_exec");
    int nerrors = arg_parse(argc, argv, (void **) &tokens_api_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, tokens_api_args.end, argv[0]);
        return 1;
    }
    return 0;
}

static struct {
    struct arg_end *end;
} call_api_args;

static int call_api_exec(int argc, char **argv) {
    ESP_LOGI(TAG, "call_api_exec");
    int nerrors = arg_parse(argc, argv, (void **) &call_api_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, call_api_args.end, argv[0]);
        return 1;
    }
    return 0;
}

void register_api() {
    ESP_LOGI(TAG, "register_api");

    login_api_args.end = arg_end(1);

    const esp_console_cmd_t login_api_cmd = {
        .command = "login",
        .help = "Login ground station to Platzi",
        .hint = NULL,
        .func = &login_api_exec,
        .argtable = &login_api_args
    };

    logout_api_args.end = arg_end(1);

    const esp_console_cmd_t logout_api_cmd = {
        .command = "logout",
        .help = "Clean login data",
        .hint = NULL,
        .func = &logout_api_exec,
        .argtable = &logout_api_args
    };

    sync_api_args.end = arg_end(1);

    const esp_console_cmd_t sync_api_cmd = {
        .command = "sync",
        .help = "Sync settings from backend",
        .hint = NULL,
        .func = &sync_api_exec,
        .argtable = &sync_api_args
    };

    account_api_args.end = arg_end(1);

    const esp_console_cmd_t account_api_cmd = {
        .command = "account",
        .help = "Show info about user and device identity",
        .hint = NULL,
        .func = &account_api_exec,
        .argtable = &account_api_args
    };

    session_api_args.end = arg_end(1);

    const esp_console_cmd_t session_api_cmd = {
        .command = "session",
        .help = "Show session status (is loggedin)",
        .hint = NULL,
        .func = &session_api_exec,
        .argtable = &session_api_args
    };

    code_api_args.end = arg_end(1);

    const esp_console_cmd_t code_api_cmd = {
        .command = "code",
        .help = "Show code to login",
        .hint = NULL,
        .func = &code_api_exec,
        .argtable = &code_api_args
    };

    tokens_api_args.end = arg_end(1);

    const esp_console_cmd_t tokens_api_cmd = {
        .command = "tokens",
        .help = "Show access and refresh tokens",
        .hint = NULL,
        .func = &tokens_api_exec,
        .argtable = &tokens_api_args
    };

    call_api_args.end = arg_end(1);

    const esp_console_cmd_t call_api_cmd = {
        .command = "call",
        .help = "Call API endpoint by name",
        .hint = NULL,
        .func = &call_api_exec,
        .argtable = &call_api_args
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&login_api_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&logout_api_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&sync_api_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&account_api_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&session_api_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&code_api_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&tokens_api_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&call_api_cmd));
}