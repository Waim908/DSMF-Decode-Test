#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <windows.h>

/* Language IDs (avoid conflict with Windows LANG_CHINESE/LANG_ENGLISH) */
#define APP_LANG_CHINESE 0
#define APP_LANG_ENGLISH 1

/* Detect system UI language, returns APP_LANG_CHINESE or APP_LANG_ENGLISH */
int Config_DetectSystemLanguage(void);

/* Configuration structure */
typedef struct {
    int language;           /* 0=Chinese, 1=English */
    wchar_t lastOpenDir[MAX_PATH]; /* Last opened file directory */
} AppConfig;

/* Initialize config with defaults and load from INI if exists */
void Config_Init(AppConfig *config);

/* Save config to INI file */
void Config_Save(const AppConfig *config);

/* Get INI file path (next to exe) */
void Config_GetIniPath(wchar_t *path, int path_len);

#endif /* APP_CONFIG_H */
