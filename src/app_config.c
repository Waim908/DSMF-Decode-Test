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
#define KEY_WINE_FIX     L"WineFix"

/* INI file version */
#define INI_VERSION      2
#define KEY_VERSION      L"Version"

/* Language string values for INI */
#define LANG_STR_CHINESE L"zh-CN"
#define LANG_STR_ENGLISH L"en-US"

/* Detect system UI language */
int Config_DetectSystemLanguage(void)
{
    LANGID langId = GetUserDefaultUILanguage();
    /* Primary language ID: Chinese variants (Simplified=0x04, Traditional=0x04 with sub) */
    WORD primaryLang = PRIMARYLANGID(langId);
    WORD subLang = SUBLANGID(langId);

    /* Simplified Chinese: 0x0804, Traditional Chinese: 0x0404, etc. */
    if (primaryLang == LANG_CHINESE) {
        return APP_LANG_CHINESE;
    }
    return APP_LANG_ENGLISH;
}

/* Get path to INI file (same directory as exe, fixed name) */
void Config_GetIniPath(wchar_t *path, int path_len)
{
    GetModuleFileNameW(NULL, path, path_len);
    /* Replace exe filename with fixed ini name */
    wchar_t *slash = wcsrchr(path, L'\\');
    if (!slash) slash = wcsrchr(path, L'/');
    if (slash) {
        wcscpy_s(slash + 1, path_len - (slash + 1 - path), L"DSMF-Decode-Test.ini");
    }
}

/* Initialize config with defaults */
static void Config_SetDefaults(AppConfig *config)
{
    config->language = Config_DetectSystemLanguage();
    config->wine_fix = 0;  /* Default: disabled */
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
    wchar_t langStr[16];
    GetPrivateProfileStringW(INI_SECTION, KEY_LANGUAGE, LANG_STR_CHINESE,
        langStr, 16, iniPath);

    /* Parse language string */
    if (wcscmp(langStr, LANG_STR_ENGLISH) == 0) {
        config->language = APP_LANG_ENGLISH;
    } else if (wcscmp(langStr, LANG_STR_CHINESE) == 0) {
        config->language = APP_LANG_CHINESE;
    } else {
        /* Legacy numeric format or invalid, use detected system language */
        config->language = Config_DetectSystemLanguage();
    }
    
    /* Read wine fix setting */
    config->wine_fix = (int)GetPrivateProfileIntW(INI_SECTION, KEY_WINE_FIX, 0, iniPath);
    
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

    /* Write language as readable string */
    WritePrivateProfileStringW(INI_SECTION, KEY_LANGUAGE,
        (config->language == APP_LANG_ENGLISH) ? LANG_STR_ENGLISH : LANG_STR_CHINESE,
        iniPath);
    
    /* Write wine fix setting */
    swprintf(value, 32, L"%d", config->wine_fix);
    WritePrivateProfileStringW(INI_SECTION, KEY_WINE_FIX, value, iniPath);
    
    /* Write last open directory */
    WritePrivateProfileStringW(INI_SECTION, KEY_LAST_OPEN_DIR,
        config->lastOpenDir, iniPath);
}
