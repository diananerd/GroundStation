#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "api_calls.h"

void initialize_api(void) {
    ESP_LOGI("api", "initialize api");
}

static struct {
    struct arg_end *end;
} clear_args;

static int clear(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &clear_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, clear_args.end, argv[0]);
        return 1;
    }
    ESP_LOGI(__func__, "Clear session tokens from NVS");
    bool err = clear_session();
    if (err) {
        ESP_LOGW(__func__, "Clear error");
        return 1;
    }
    ESP_LOGI(__func__, "clear end");
    return 0;
} 

static struct {
    struct arg_int *timeout;
    struct arg_str *url;
    struct arg_end *end;
} get_args;

static int get(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &get_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, get_args.end, argv[0]);
        return 1;
    }
    ESP_LOGI(__func__, "get to '%s'", get_args.url->sval[0]);
    bool err = get_url(get_args.url->sval[0], get_args.timeout->ival[0]);
    if (err) {
        ESP_LOGW(__func__, "Connection timed out");
        return 1;
    }
    ESP_LOGI(__func__, "get end");
    return 0;
}

struct {
    struct arg_int *timeout;
    struct arg_str *url;
    struct arg_str *body;
    struct arg_end *end;
} post_args;

static int post(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &post_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, post_args.end, argv[0]);
        return 1;
    }
    ESP_LOGI(__func__, "post %s to '%s'", post_args.body->sval[0], post_args.url->sval[0]);
    char res[128] = "";
    bool err = http_post(post_args.url->sval[0], post_args.body->sval[0], res);
    if (err) {
        ESP_LOGW(__func__, "Connection timed out");
        return 1;
    }
    printf("res: %s\n", res);
    ESP_LOGI(__func__, "post end");
    return 0;
}

static struct {
    struct arg_end *end;
} sync_args;

static int sync_handler(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &sync_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, sync_args.end, argv[0]);
        return 1;
    }
    ESP_LOGI(__func__, "Sync account");
    bool err = sync_account();
    if (err) {
        ESP_LOGW(__func__, "Connection timed out");
        return 1;
    }
    ESP_LOGI(__func__, "Sync end");
    return 0;
}

void register_api(void) {
    clear_args.end = arg_end(2);

    const esp_console_cmd_t clear_cmd = {
        .command = "clear",
        .help = "Clear session tokens from storage",
        .hint = NULL,
        .func = &clear,
        .argtable = &clear_args
    };

    get_args.url = arg_str1(NULL, NULL, "<url>", "Site url");
    get_args.timeout = arg_int0("t", "timeout", "<t>", "Connection timeout, ms");
    get_args.end = arg_end(2);

    const esp_console_cmd_t get_cmd = {
        .command = "get",
        .help = "Get a website by url",
        .hint = NULL,
        .func = &get,
        .argtable = &get_args
    };

    post_args.url = arg_str1(NULL, NULL, "<url>", "Site url");
    post_args.body = arg_str1("b", "body", "<body>", "Body string");
    post_args.timeout = arg_int0("t", "timeout", "<t>", "Connection timeout, ms");
    post_args.end = arg_end(2);

    const esp_console_cmd_t post_cmd = {
        .command = "post",
        .help = "Send data to website by url",
        .hint = NULL,
        .func = &post,
        .argtable = &post_args
    };

    sync_args.end = arg_end(2);

    const esp_console_cmd_t sync_cmd = {
        .command = "sync",
        .help = "Sync account and get session tokens",
        .hint = NULL,
        .func = &sync_handler,
        .argtable = &sync_args
    };

    ESP_ERROR_CHECK( esp_console_cmd_register(&clear_cmd) );
    ESP_ERROR_CHECK( esp_console_cmd_register(&get_cmd) );
    ESP_ERROR_CHECK( esp_console_cmd_register(&post_cmd) );
    ESP_ERROR_CHECK( esp_console_cmd_register(&sync_cmd) );
}
