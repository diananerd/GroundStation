#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_console.h"
#include "argtable3/argtable3.h"
#include "settings.h"
#include "settings_console.h"

static const char* TAG = "SETTINGS_CONSOLE";

static struct {
    struct arg_lit *json;
    struct arg_end *end;
} list_settings_args;

static int list_settings_exec(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &list_settings_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, list_settings_args.end, argv[0]);
        return 1;
    }

    if (list_settings_args.json->count > 0) {
        settings_list_json();
    } else {
        settings_list();
    }
    return 0;
}

static struct {
    struct arg_str *key;
    struct arg_end *end;
} get_settings_args;

static int get_settings_exec(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &get_settings_args);
        if (nerrors != 0) {
        arg_print_errors(stderr, get_settings_args.end, argv[0]);
        return 1;
    }

    if (get_settings_args.key->count == 0) {
        ESP_LOGI(TAG, "%s not found", get_settings_args.key->sval[0]);
        return 1;
    }

    setting_t setting = {};
    setting.key = get_settings_args.key->sval[0];

    settings_handle_t settings_handle;
    settings_create(&settings_handle);

    esp_err_t err = settings_get(&settings_handle, &setting);
    if (err != ESP_OK) {
        printf("%s not found", get_settings_args.key->sval[0]);
        return 0;
    }

    if (setting.type == STRING) {
        printf("%s\n", setting.valuestring);
    } else if (setting.type == NUMBER) {
        printf("%i\n", setting.valueint);
    } else {
        printf("invalid type\n");
    }

    return 0;
}

static struct {
    struct arg_str *key;
    struct arg_str *value;
    struct arg_end *end;
} set_settings_args;

static int set_settings_exec(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &set_settings_args);
        if (nerrors != 0) {
        arg_print_errors(stderr, set_settings_args.end, argv[0]);
        return 1;
    }

    if (set_settings_args.key->count == 0) {
        ESP_LOGI(TAG, "key not found");
        return 1;
    }

    if (set_settings_args.value->count == 0) {
        ESP_LOGI(TAG, "value not found");
        return 1;
    }

    setting_t setting = {};
    setting.key = set_settings_args.key->sval[0];

    int valueint = atoi(set_settings_args.value->sval[0]);
    if (strcmp(set_settings_args.value->sval[0], "0") == 0 || valueint != 0) {
        ESP_LOGI(TAG, "number %s=%s", set_settings_args.key->sval[0], set_settings_args.value->sval[0]);
        setting.type = NUMBER;
        setting.valueint = valueint;
    } else {
        ESP_LOGI(TAG, "string %s=%s", set_settings_args.key->sval[0], set_settings_args.value->sval[0]);
        setting.type = STRING;
        setting.valuestring = set_settings_args.value->sval[0];
    }

    settings_handle_t settings_handle;
    settings_create(&settings_handle);

    settings_set(&settings_handle, &setting);
    printf("%s\n", set_settings_args.value->sval[0]);

    return 0;
}

static struct {
    struct arg_str *key;
    struct arg_end *end;
} delete_settings_args;

static int delete_settings_exec(int argc, char **argv) {
    int nerrors = arg_parse(argc, argv, (void **) &delete_settings_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, delete_settings_args.end, argv[0]);
        return 1;
    }

    if (delete_settings_args.key->count == 0) {
        ESP_LOGI(TAG, "key not found");
        return 1;
    }

    ESP_LOGI(TAG, "delete %s", delete_settings_args.key->sval[0]);

    setting_t setting = {};
    setting.key = delete_settings_args.key->sval[0];

    settings_handle_t settings_handle;
    settings_create(&settings_handle);

    settings_delete(&settings_handle, &setting);

    return 0;
}

void register_settings() {
    ESP_LOGI(TAG, "register_settings");

    list_settings_args.json = arg_lit0("j", "json", "show result as json");
    list_settings_args.end = arg_end(1);

    const esp_console_cmd_t list_settings_cmd = {
        .command = "settings",
        .help = "list all settings",
        .hint = NULL,
        .func = &list_settings_exec,
        .argtable = &list_settings_args
    };

    get_settings_args.key = arg_str1(NULL, NULL, "key", "get setting value by key");
    get_settings_args.end = arg_end(1);

    const esp_console_cmd_t get_settings_cmd = {
        .command = "get",
        .help = "get setting by key",
        .hint = NULL,
        .func = &get_settings_exec,
        .argtable = &get_settings_args
    };

    set_settings_args.key = arg_str1(NULL, NULL, "key", "setting key");
    set_settings_args.value = arg_str1(NULL, NULL, "value", "setting value");
    set_settings_args.end = arg_end(2);

    const esp_console_cmd_t set_settings_cmd = {
        .command = "set",
        .help = "set setting by key and value",
        .hint = NULL,
        .func = &set_settings_exec,
        .argtable = &set_settings_args
    };

    delete_settings_args.key = arg_str1(NULL, NULL, "key", "setting key or name");
    delete_settings_args.end = arg_end(1);

    const esp_console_cmd_t delete_settings_cmd = {
        .command = "delete",
        .help = "delete setting by key",
        .hint = NULL,
        .func = &delete_settings_exec,
        .argtable =&delete_settings_args
    };

    ESP_ERROR_CHECK(esp_console_cmd_register(&list_settings_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&get_settings_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&set_settings_cmd));
    ESP_ERROR_CHECK(esp_console_cmd_register(&delete_settings_cmd));
}
