#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Register API Calls functions
void nvs_session_init();
bool clear_session();
bool get_url(const char *url, int timeout_ms);
bool http_get(const char *url, char* res);
bool http_post(const char *url, const char *body, char* res);
bool sync_account();

#ifdef __cplusplus
}
#endif
