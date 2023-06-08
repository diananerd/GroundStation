#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Register API Calls functions
void nvs_session_init();
bool clear_session();
bool get_url(const char *url, int timeout_ms);
bool sync_account();

#ifdef __cplusplus
}
#endif
