/*
 * Application configuration - INI file management
 */
#include "app_config.h"
#include <stdio.h>
#include <shlwapi.h>

/* INI file section and key names */
#define INI_SECTION      L"Settings"
#define KEY_LANGUAGE     L"Language"
#define KEY_LAST_OPEN_DIR L"LastOpenDir"

/* INI file version for future updates */
#define INI_VERSION      1
#define KEY_VERSION      L"Version"

/* Get path to INI file (same directory as exe) */
void Config_GetIniPath(wchar_t *path, int path_len)
{
    GetModuleFileNameW(NULL, path, path_len);
    /* Replace .exe extension with .ini */
    wchar_t *dot = wcsrchr(path, L'.');
    if (dot) {
        wcscpy_s(dot, path_len - (dot - path), L".ini");
    }
}

/* Initialize config with defaults */
static void Config_SetDefaults(AppConfig *config)
{
    config->language = APP_LANG_CHINESE;
    config->lastOpenDir[0] = L'\0';
}

/* Load config from INI file */
void Config_Init(AppConfig *config)
{
    wchar_t iniPath[MAX_PATH];
    
    /* Set defaults first */
    Config_SetDefaults(config);
    
    /* Get INI path */
    Config_GetIniPath(iniPath, MAX_PATH);
    
    /* Check if INI file exists */
    DWORD attr = GetFileAttributesW(iniPath);
    if (attr == INVALID_FILE_ATTRIBUTES) {
        /* INI doesn't exist, create it with defaults */
        Config_Save(config);
        return;
    }
    
    /* Read version for future compatibility */
    int version = (int)GetPrivateProfileIntW(INI_SECTION, KEY_VERSION, 0, iniPath);
    
    /* Read settings */
    config->language = (int)GetPrivateProfileIntW(INI_SECTION, KEY_LANGUAGE, APP_LANG_CHINESE, iniPath);
    
    /* Validate language */
    if (config->language != APP_LANG_CHINESE && config->language != APP_LANG_ENGLISH) {
        config->language = APP_LANG_CHINESE;
    }
    
    /* Read last open directory */
    GetPrivateProfileStringW(INI_SECTION, KEY_LAST_OPEN_DIR, L"",
        config->lastOpenDir, MAX_PATH, iniPath);
    
    /* If last open dir is empty, use exe directory */
    if (config->lastOpenDir[0] == L'\0') {
        GetModuleFileNameW(NULL, config->lastOpenDir, MAX_PATH);
        PathRemoveFileSpecW(config->lastOpenDir);
    }
    
    /* If directory doesn't exist, fall back to exe directory */
    if (!PathFileExistsW(config->lastOpenDir)) {
        GetModuleFileNameW(NULL, config->lastOpenDir, MAX_PATH);
        PathRemoveFileSpecW(config->lastOpenDir);
    }
    
    /* Save to update version if needed */
    if (version < INI_VERSION) {
        Config_Save(config);
    }
}

/* Save config to INI file */
void Config_Save(const AppConfig *config)
{
    wchar_t iniPath[MAX_PATH];
    wchar_t value[32];
    
    Config_GetIniPath(iniPath, MAX_PATH);
    
    /* Write version */
    swprintf(value, 32, L"%d", INI_VERSION);
    WritePrivateProfileStringW(INI_SECTION, KEY_VERSION, value, iniPath);
    
    /* Write language */
    swprintf(value, 32, L"%d", config->language);
    WritePrivateProfileStringW(INI_SECTION, KEY_LANGUAGE, value, iniPath);
    
    /* Write last open directory */
    WritePrivateProfileStringW(INI_SECTION, KEY_LAST_OPEN_DIR,
        config->lastOpenDir, iniPath);
}
