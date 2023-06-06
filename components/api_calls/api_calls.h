#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Register API Calls functions
bool ping_url(const char *url, int timeout_ms);

#ifdef __cplusplus
}
#endif
