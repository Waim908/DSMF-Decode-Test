/*
 * Language support - Chinese and English strings
 */
#include "lang.h"

/* Current language */
static int g_currentLang = LANG_CHINESE;

/* Chinese strings */
static const LangStrings g_chinese = {
    /* Window title */
    L"DirectShowMediaFoundationDecodeTest",
    
    /* Buttons */
    L"DirectShow 播放",
    L"DS + DXVA2",
    L"MF 软件解码",
    L"MF + DXVA2",
    L"MF + D3D11",
    L"MF + D3D12",
    L"停止播放",
    L"打开文件...",
    L"关于...",
    L"确定",
    
    /* Status messages */
    L"请先打开视频文件",
    L"已停止",
    L"已选择文件: %ls",
    L"正在播放 %ls",
    L"播放完成",
    L"解码错误",
    L"正在打开...",
    L"正在初始化...",
    L"硬件加速不可用，将使用软件解码",
    L"设备初始化失败，将使用软件解码",
    L"纯音频",
    
    /* Hardware tags */
    L"软解",
    L"DXVA2硬解",
    L"D3D11硬解",
    L"D3D12(软解)",
    L"硬件加速",
    L"软件回退",
    
    /* Dialog messages */
    L"请先选择视频文件",
    L"文件不存在：\n%ls",
    L"无法播放 - %ls",
    L"播放失败",
    L"错误",
    L"提示",
    
    /* About dialog */
    L"关于 DSMF-Decode-Test",
    L"版本：0.1",
    
    /* File dialog */
    L"选择视频文件",
    L"视频文件",
    L"所有文件"
};

/* English strings */
static const LangStrings g_english = {
    /* Window title */
    L"DirectShowMediaFoundationDecodeTest",
    
    /* Buttons */
    L"DirectShow Play",
    L"DS + DXVA2",
    L"MF Software",
    L"MF + DXVA2",
    L"MF + D3D11",
    L"MF + D3D12",
    L"Stop",
    L"Open File...",
    L"About...",
    L"OK",
    
    /* Status messages */
    L"Please open a video file first",
    L"Stopped",
    L"File selected: %ls",
    L"Playing %ls",
    L"Playback complete",
    L"Decode error",
    L"Opening...",
    L"Initializing...",
    L"HW acceleration unavailable, using software decode",
    L"Device init failed, using software decode",
    L"Audio only",
    
    /* Hardware tags */
    L"Software",
    L"DXVA2 HW",
    L"D3D11 HW",
    L"D3D12 (SW)",
    L"Hardware",
    L"Software fallback",
    
    /* Dialog messages */
    L"Please select a video file first",
    L"File not found:\n%ls",
    L"Cannot play - %ls",
    L"Playback failed",
    L"Error",
    L"Hint",
    
    /* About dialog */
    L"About DSMF-Decode-Test",
    L"Version: 0.1",
    
    /* File dialog */
    L"Select Video File",
    L"Video Files",
    L"All Files"
};

/* Get strings for current language */
const LangStrings* Lang_GetStrings(void)
{
    return (g_currentLang == APP_LANG_ENGLISH) ? &g_english : &g_chinese;
}

/* Get current language */
int Lang_GetCurrent(void)
{
    return g_currentLang;
}

/* Set current language */
void Lang_SetCurrent(int lang)
{
    if (lang == APP_LANG_CHINESE || lang == APP_LANG_ENGLISH) {
        g_currentLang = lang;
    }
}