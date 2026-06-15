/*
 * Windows Video Decoder Test
 * Main GUI - Win32 window with buttons for different decoding methods.
 */
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <stdio.h>
#include <string.h>

#include "resource.h"
#include "app_config.h"
#include "lang.h"
#include "log.h"
#include "directshow_decoder.h"
#include "mf_decoder.h"
#include "dxva2_helper.h"
#include "d3d11_video_helper.h"
#include "d3d12_video_helper.h"

/* Compiler info macros */
#ifdef _MSC_VER
#define COMPILER_INFO_FMT L"MSVC %d.%d"
#define COMPILER_INFO_ARGS _MSC_VER / 100, _MSC_VER % 100
#elif defined(__MINGW64__)
#define COMPILER_INFO_FMT L"MinGW-w64 %d.%d (GCC %d.%d.%d)"
#define COMPILER_INFO_ARGS __MINGW64_VERSION_MAJOR, __MINGW64_VERSION_MINOR, \
    __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__
#elif defined(__MINGW32__)
#define COMPILER_INFO_FMT L"MinGW %d.%d (GCC %d.%d.%d)"
#define COMPILER_INFO_ARGS __MINGW32_MAJOR_VERSION, __MINGW32_MINOR_VERSION, \
    __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__
#else
#define COMPILER_INFO_FMT L"GCC %d.%d.%d"
#define COMPILER_INFO_ARGS __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__
#endif

/* Architecture detection */
#if defined(_M_AMD64) || defined(__x86_64__) || defined(__amd64__)
#define ARCH_STR L"x64"
#elif defined(_M_IX86) || defined(__i386__)
#define ARCH_STR L"x86"
#elif defined(_M_ARM64) || defined(__aarch64__)
#define ARCH_STR L"ARM64"
#else
#define ARCH_STR L"Unknown"
#endif

/* Window class names */
static const wchar_t CLASS_NAME[] = L"VideoDecoderTestClass";

/* Button layout constants */
#define BTN_H       30
#define BTN_GAP     6
#define BTN_PAD     8
#define ROW1_Y      BTN_PAD
#define ROW2_Y      (BTN_PAD + BTN_H + BTN_GAP)
#define ROW3_Y      (BTN_PAD + BTN_H + BTN_GAP + BTN_H + BTN_GAP)
#define TOOLBAR_H   (BTN_PAD + BTN_H + BTN_GAP + BTN_H + BTN_GAP + BTN_H + BTN_PAD)

/* Global controls */
static HWND g_hwndMain       = NULL;
static HWND g_hwndBtnDS      = NULL;
static HWND g_hwndBtnDSDxva2 = NULL;
static HWND g_hwndBtnMF      = NULL;
static HWND g_hwndBtnDxva2   = NULL;
static HWND g_hwndBtnD3D11   = NULL;
static HWND g_hwndBtnD3D12   = NULL;
static HWND g_hwndBtnStop    = NULL;
static HWND g_hwndBtnOpen    = NULL;
static HWND g_hwndBtnAbout   = NULL;
static HWND g_hwndBtnSettings = NULL;
static HWND g_hwndBtnClearLog = NULL;
static HWND g_hwndBtnExportLog = NULL;
static HWND g_hwndDisplay    = NULL;
HWND g_hwndLog               = NULL;
static HWND g_hwndStatus     = NULL;
static HFONT g_hFont         = NULL;

/* Current state */
static int   g_currentMode   = 0;
static int   g_renderTimerActive = 0;
static int   g_switching      = 0;  /* Guard against rapid button clicks */
static wchar_t g_filePath[MAX_PATH] = {0};
static HBRUSH g_hBrushBlack  = NULL;

/* Application config */
static AppConfig g_config;

/* Forward declarations */
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static void CreateControls(HWND hwnd);
static void UpdateStatus(const wchar_t *fmt, ...);
static void StartDirectShow(void);
static void StartDirectShowDXVA2(void);
static void StartMFSoftware(void);
static void StartMFDXVA2(void);
static void StartMFD3D11(void);
static void StartMFD3D12(void);
static void StopAll(void);
static void ResizeControls(HWND hwnd);
static int  OpenFileDialog(HWND hwnd, wchar_t *path, int path_len);
static void ShowAboutDialog(HWND hwndParent);
static INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static void ShowSettingsDialog(HWND hwndParent);
static INT_PTR CALLBACK SettingsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

/* Helper: create a button */
static HWND MakeButton(HWND parent, const wchar_t *text, int id, int x, int y, int w)
{
    HWND h = CreateWindowW(L"BUTTON", text,
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT,
        x, y, w, BTN_H, parent, (HMENU)(UINT_PTR)id, NULL, NULL);
    if (h && g_hFont) SendMessageW(h, WM_SETFONT, (WPARAM)g_hFont, TRUE);
    return h;
}

/* Enable/disable all playback buttons to prevent rapid-click race conditions */
static void EnableButtons(int enable)
{
    EnableWindow(g_hwndBtnDS,      enable);
    EnableWindow(g_hwndBtnDSDxva2, enable);
    EnableWindow(g_hwndBtnMF,      enable);
    EnableWindow(g_hwndBtnDxva2,   enable);
    EnableWindow(g_hwndBtnD3D11,   enable);
    EnableWindow(g_hwndBtnD3D12,   enable);
    EnableWindow(g_hwndBtnOpen,    enable);
}

/* Check if a file has been selected, show message if not */
static int CheckFileSelected(void)
{
    const LangStrings *lang = Lang_GetStrings();
    if (g_filePath[0] == L'\0') {
        MessageBoxW(g_hwndMain, lang->msgSelectFile, lang->msgHint, MB_OK | MB_ICONINFORMATION);
        return 0;
    }
    return 1;
}

/* Check if the selected file actually exists on disk */
static int CheckFileExists(void)
{
    const LangStrings *lang = Lang_GetStrings();
    DWORD attr = GetFileAttributesW(g_filePath);
    if (attr == INVALID_FILE_ATTRIBUTES || (attr & FILE_ATTRIBUTE_DIRECTORY)) {
        wchar_t msg[512];
        swprintf(msg, 512, lang->msgFileNotFound, g_filePath);
        MessageBoxW(g_hwndMain, msg, lang->msgError, MB_OK | MB_ICONERROR);
        return 0;
    }
    return 1;
}

/* Entry point */
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPWSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEXW wc = {0};
    MSG msg;
    HRESULT hr;
    const LangStrings *lang;

    /* Initialize config and language */
    Config_Init(&g_config);
    Lang_SetCurrent(g_config.language);
    ds_set_wine_fix(g_config.wine_fix);
    lang = Lang_GetStrings();

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr)) {
        MessageBoxW(NULL, L"COM initialization failed", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(icex);
    icex.dwICC  = ICC_BAR_CLASSES | ICC_STANDARD_CLASSES | ICC_LINK_CLASS;
    InitCommonControlsEx(&icex);

    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance      = hInstance;
    wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground  = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName  = CLASS_NAME;
    wc.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wc.hIconSm        = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    RegisterClassExW(&wc);

    /* Build window title with architecture info */
    wchar_t windowTitle[256];
    swprintf(windowTitle, 256, L"%ls (%ls)", lang->windowTitle, ARCH_STR);

    g_hwndMain = CreateWindowExW(
        0, CLASS_NAME, windowTitle,
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, 960, 640,
        NULL, NULL, hInstance, NULL);

    if (!g_hwndMain) {
        MessageBoxW(NULL, L"Window creation failed", L"Error", MB_OK | MB_ICONERROR);
        CoUninitialize();
        return 1;
    }

    ShowWindow(g_hwndMain, nCmdShow);
    UpdateWindow(g_hwndMain);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    StopAll();
    CoUninitialize();
    return (int)msg.wParam;
}

/* Create child controls */
static void CreateControls(HWND hwnd)
{
    const LangStrings *lang = Lang_GetStrings();

    /* Common font */
    g_hFont = CreateFontW(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                          CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

    /* Row 1: DirectShow methods */
    int bw = 140;
    int x = BTN_PAD;
    g_hwndBtnDS      = MakeButton(hwnd, lang->btnDirectShow,  IDC_BTN_DIRECTSHOW,  x, ROW1_Y, bw);
    x += bw + BTN_GAP;
    g_hwndBtnDSDxva2 = MakeButton(hwnd, lang->btnDsDxva2,    IDC_BTN_DS_DXVA2,    x, ROW1_Y, bw);
    x += bw + BTN_GAP;
    g_hwndBtnClearLog = MakeButton(hwnd, lang->btnClearLog,  IDC_BTN_CLEAR_LOG,   x, ROW1_Y, bw);
    x += bw + BTN_GAP;
    g_hwndBtnExportLog = MakeButton(hwnd, lang->btnExportLog, IDC_BTN_EXPORT_LOG,  x, ROW1_Y, bw);

    /* Row 2: Media Foundation methods */
    x = BTN_PAD;
    g_hwndBtnMF      = MakeButton(hwnd, lang->btnMfSoftware, IDC_BTN_MF_SOFTWARE, x, ROW2_Y, bw);
    x += bw + BTN_GAP;
    g_hwndBtnDxva2   = MakeButton(hwnd, lang->btnMfDxva2,   IDC_BTN_MF_DXVA2,    x, ROW2_Y, bw);
    x += bw + BTN_GAP;
    g_hwndBtnD3D11   = MakeButton(hwnd, lang->btnMfD3d11,   IDC_BTN_MF_D3D11,    x, ROW2_Y, bw);
    x += bw + BTN_GAP;
    g_hwndBtnD3D12   = MakeButton(hwnd, lang->btnMfD3d12,   IDC_BTN_MF_D3D12,    x, ROW2_Y, bw);

    /* Row 3: control */
    x = BTN_PAD;
    g_hwndBtnStop    = MakeButton(hwnd, lang->btnStop,       IDC_BTN_STOP,        x, ROW3_Y, bw);
    x += bw + BTN_GAP;
    g_hwndBtnOpen    = MakeButton(hwnd, lang->btnOpenFile,   IDC_BTN_OPEN_FILE,   x, ROW3_Y, bw);
    x += bw + BTN_GAP;
    g_hwndBtnAbout   = MakeButton(hwnd, lang->btnAbout,      IDC_BTN_ABOUT,       x, ROW3_Y, bw);
    x += bw + BTN_GAP;
    g_hwndBtnSettings = MakeButton(hwnd, lang->btnSettings, IDC_BTN_SETTINGS, x, ROW3_Y, bw);

    /* Log display area (right of buttons) */
    g_hwndLog = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL |
        ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
        0, 0, 0, 0,
        hwnd, (HMENU)(UINT_PTR)IDC_LOG_DISPLAY, NULL, NULL);
    if (g_hwndLog && g_hFont) SendMessageW(g_hwndLog, WM_SETFONT, (WPARAM)g_hFont, TRUE);

    /* Video display area - SS_BLACKRECT ensures black background even during resize */
    g_hwndDisplay = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | SS_BLACKRECT | WS_CLIPSIBLINGS,
        0, 0, 0, 0,
        hwnd, (HMENU)(UINT_PTR)IDC_VIDEO_DISPLAY, NULL, NULL);

    /* Status bar */
    g_hwndStatus = CreateWindowExW(
        0, STATUSCLASSNAMEW, lang->statusReady,
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0,
        hwnd, (HMENU)(UINT_PTR)IDC_STATUS_BAR, NULL, NULL);
}

/* Resize controls */
static void ResizeControls(HWND hwnd)
{
    RECT rc;
    GetClientRect(hwnd, &rc);

    /* Freeze drawing to prevent flickering/tearing during rapid resize (e.g. maximize) */
    SendMessageW(hwnd, WM_SETREDRAW, FALSE, 0);

    /* Status bar */
    SendMessageW(g_hwndStatus, WM_SIZE, 0, 0);
    RECT rcStatus;
    GetWindowRect(g_hwndStatus, &rcStatus);
    int status_h = rcStatus.bottom - rcStatus.top;

    /* Calculate button area width (Row2 is widest: 4 buttons * 140 + 3 gaps + padding) */
    int btn_area_w = 4 * 140 + 3 * BTN_GAP + BTN_PAD;

    /* Log display area (right of buttons, same height as toolbar) */
    int log_x = btn_area_w + BTN_GAP;
    int log_w = rc.right - log_x - BTN_PAD;
    if (log_w < 100) log_w = 100;
    MoveWindow(g_hwndLog, log_x, BTN_PAD, log_w, TOOLBAR_H - BTN_PAD * 2, FALSE);

    /* Video display fills the rest */
    int display_y = TOOLBAR_H;
    int display_h = rc.bottom - display_y - status_h;
    if (display_h < 0) display_h = 0;

    MoveWindow(g_hwndDisplay, 0, display_y, rc.right, display_h, FALSE);

    /* Unfreeze drawing and repaint all at once */
    SendMessageW(hwnd, WM_SETREDRAW, TRUE, 0);
    RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);

    /* Clear display area to prevent residual frames */
    if (g_hwndDisplay) {
        HDC hdc = GetDC(g_hwndDisplay);
        if (hdc) {
            RECT vrc;
            GetClientRect(g_hwndDisplay, &vrc);
            FillRect(hdc, &vrc, (HBRUSH)GetStockObject(BLACK_BRUSH));
            ReleaseDC(g_hwndDisplay, hdc);
        }
    }

    /* DirectShow resize */
    if (g_currentMode == 1 || g_currentMode == 5) {
        RECT vrc;
        GetClientRect(g_hwndDisplay, &vrc);
        ds_resize(0, 0, vrc.right, vrc.bottom);
    }
}

static void UpdateStatus(const wchar_t *fmt, ...)
{
    wchar_t msg[512];
    va_list args;
    va_start(args, fmt);
    vswprintf(msg, 512, fmt, args);
    va_end(args);

    if (g_hwndStatus)
        SendMessageW(g_hwndStatus, SB_SETTEXTW, 0, (LPARAM)msg);
}

/* Append text to log display with auto-scroll */
void Log_Append(const wchar_t *text)
{
    if (!g_hwndLog) return;

    /* Get current text length */
    int len = GetWindowTextLengthW(g_hwndLog);

    /* Limit text length to prevent excessive memory usage (keep last 64KB) */
    if (len > 65536) {
        /* Select and delete first half */
        SendMessageW(g_hwndLog, EM_SETSEL, 0, len / 2);
        SendMessageW(g_hwndLog, EM_REPLACESEL, FALSE, (LPARAM)L"");
        len = GetWindowTextLengthW(g_hwndLog);
    }

    /* Move caret to end */
    SendMessageW(g_hwndLog, EM_SETSEL, len, len);

    /* Append new text */
    SendMessageW(g_hwndLog, EM_REPLACESEL, FALSE, (LPARAM)text);
}

/* Log printf-style formatted text */
void Log_Printf(const wchar_t *fmt, ...)
{
    wchar_t msg[1024];
    va_list args;
    va_start(args, fmt);
    vswprintf(msg, 1024, fmt, args);
    va_end(args);

    /* Append with newline */
    wcscat_s(msg, 1024, L"\r\n");
    Log_Append(msg);
}

static void StopAll(void)
{
    const LangStrings *lang = Lang_GetStrings();
    MSG msg;
    
    if (g_renderTimerActive) {
        KillTimer(g_hwndMain, TIMER_RENDER);
        g_renderTimerActive = 0;
    }
    ds_stop();
    mf_stop();
    dxva2_cleanup();
    d3d11_video_cleanup();
    d3d12_video_cleanup();
    g_currentMode = 0;

    /* Give the system time to fully release COM objects and resources.
     * This prevents issues when rapidly switching between decoders. */
    Sleep(20);
    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    /* Clear display area to prevent residual frames. */
    if (g_hwndDisplay) {
        HDC hdc = GetDC(g_hwndDisplay);
        if (hdc) {
            RECT rc;
            GetClientRect(g_hwndDisplay, &rc);
            FillRect(hdc, &rc, (HBRUSH)GetStockObject(BLACK_BRUSH));
            ReleaseDC(g_hwndDisplay, hdc);
        }
        RedrawWindow(g_hwndDisplay, NULL, NULL,
            RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    }
    UpdateStatus(lang->statusStopped);
}

static void StartDirectShow(void)
{
    const LangStrings *lang = Lang_GetStrings();
    wchar_t msg[512];
    if (g_switching) return;
    if (!CheckFileSelected()) return;
    if (!CheckFileExists()) return;
    g_switching = 1;
    EnableButtons(FALSE);
    StopAll();
    UpdateStatus(L"DirectShow: %ls", lang->statusOpening);
    int ret = ds_open(g_filePath, g_hwndDisplay);
    if (ret != 0) {
        swprintf(msg, 512, L"DirectShow: %ls", g_filePath);
        UpdateStatus(L"%ls", msg);
        EnableButtons(TRUE);
        g_switching = 0;
        MessageBoxW(g_hwndMain, msg, lang->msgError, MB_OK | MB_ICONERROR);
        return;
    }
    ret = ds_play();
    if (ret != 0) { UpdateStatus(L"DirectShow: %ls", lang->msgPlayFailed); ds_stop(); EnableButtons(TRUE); g_switching = 0; return; }
    g_currentMode = 1;
    swprintf(msg, 512, L"DirectShow [%ls]: %ls %ls", ds_get_renderer_name(), lang->statusPlaying, g_filePath);
    UpdateStatus(L"%ls", msg);
    SetTimer(g_hwndMain, TIMER_RENDER, 100, NULL);
    g_renderTimerActive = 1;
    EnableButtons(TRUE);
    g_switching = 0;
}

static void StartDirectShowDXVA2(void)
{
    const LangStrings *lang = Lang_GetStrings();
    wchar_t msg[512];
    if (g_switching) return;
    if (!CheckFileSelected()) return;
    if (!CheckFileExists()) return;
    g_switching = 1;
    EnableButtons(FALSE);
    StopAll();
    UpdateStatus(L"DirectShow + DXVA2: %ls", lang->statusOpening);
    int ret = ds_open_dxva2(g_filePath, g_hwndDisplay, 1);
    if (ret != 0) {
        swprintf(msg, 512, L"DirectShow + DXVA2: %ls - %ls", lang->msgCannotPlay, g_filePath);
        UpdateStatus(L"%ls", msg);
        EnableButtons(TRUE);
        g_switching = 0;
        MessageBoxW(g_hwndMain, msg, lang->msgError, MB_OK | MB_ICONERROR);
        return;
    }
    ret = ds_play();
    if (ret != 0) { UpdateStatus(L"DirectShow + DXVA2: %ls", lang->msgPlayFailed); ds_stop(); EnableButtons(TRUE); g_switching = 0; return; }
    g_currentMode = 5;
    swprintf(msg, 512, L"DirectShow + DXVA2 [%ls]: %ls %ls%s", ds_get_renderer_name(), lang->statusPlaying, g_filePath,
             ds_is_using_dxva2() ? lang->hwAccel : lang->hwFallback);
    UpdateStatus(L"%ls", msg);
    SetTimer(g_hwndMain, TIMER_RENDER, 100, NULL);
    g_renderTimerActive = 1;
    EnableButtons(TRUE);
    g_switching = 0;
}

static void StartMFSoftware(void)
{
    const LangStrings *lang = Lang_GetStrings();
    wchar_t msg[512];
    if (g_switching) return;
    if (!CheckFileSelected()) return;
    if (!CheckFileExists()) return;
    g_switching = 1;
    EnableButtons(FALSE);
    StopAll();
    UpdateStatus(L"Media Foundation (%ls): %ls", lang->hwSoftware, lang->statusOpening);
    int ret = mf_open(g_filePath, g_hwndDisplay, 0);
    if (ret != 0) {
        swprintf(msg, 512, L"Media Foundation: %ls - %ls", lang->msgCannotPlay, g_filePath);
        UpdateStatus(L"%ls", msg);
        EnableButtons(TRUE);
        g_switching = 0;
        MessageBoxW(g_hwndMain, msg, lang->msgError, MB_OK | MB_ICONERROR);
        return;
    }
    g_currentMode = 2;
    mf_render_next_frame();
    swprintf(msg, 512, L"Media Foundation [%ls] (%ls): %ls", mf_get_renderer_name(), lang->hwSoftware, mf_get_decoder_info());
    UpdateStatus(L"%ls", msg);
    SetTimer(g_hwndMain, TIMER_RENDER, 33, NULL);
    g_renderTimerActive = 1;
    EnableButtons(TRUE);
    g_switching = 0;
}

static void StartMFDXVA2(void)
{
    const LangStrings *lang = Lang_GetStrings();
    wchar_t msg[512];
    if (g_switching) return;
    if (!CheckFileSelected()) return;
    if (!CheckFileExists()) return;
    g_switching = 1;
    EnableButtons(FALSE);
    StopAll();
    UpdateStatus(L"Media Foundation + DXVA2: %ls", lang->statusInit);
    if (!dxva2_check_support())
        UpdateStatus(L"DXVA2: %ls", lang->statusHwUnavailable);
    int ret = mf_open(g_filePath, g_hwndDisplay, 1);
    if (ret != 0) {
        swprintf(msg, 512, L"MF+DXVA2: %ls - %ls", lang->msgCannotPlay, g_filePath);
        UpdateStatus(L"%ls", msg);
        EnableButtons(TRUE);
        g_switching = 0;
        MessageBoxW(g_hwndMain, msg, lang->msgError, MB_OK | MB_ICONERROR);
        return;
    }
    g_currentMode = 3;
    mf_render_next_frame();
    swprintf(msg, 512, L"Media Foundation + DXVA2 [%ls]: %ls", mf_get_renderer_name(), mf_get_decoder_info());
    UpdateStatus(L"%ls", msg);
    SetTimer(g_hwndMain, TIMER_RENDER, 33, NULL);
    g_renderTimerActive = 1;
    EnableButtons(TRUE);
    g_switching = 0;
}

static void StartMFD3D11(void)
{
    const LangStrings *lang = Lang_GetStrings();
    wchar_t msg[512];
    if (g_switching) return;
    if (!CheckFileSelected()) return;
    if (!CheckFileExists()) return;
    g_switching = 1;
    EnableButtons(FALSE);
    StopAll();
    UpdateStatus(L"Media Foundation + D3D11: %ls", lang->statusInit);
    if (!d3d11_video_check_support())
        UpdateStatus(L"D3D11: %ls", lang->statusHwUnavailable);
    if (d3d11_video_init(g_hwndDisplay, 1920, 1080) != 0)
        UpdateStatus(L"D3D11: %ls", lang->statusHwInitFailed);
    int ret = mf_open(g_filePath, g_hwndDisplay, 2);
    if (ret != 0) {
        swprintf(msg, 512, L"MF+D3D11: %ls - %ls", lang->msgCannotPlay, g_filePath);
        UpdateStatus(L"%ls", msg);
        EnableButtons(TRUE);
        g_switching = 0;
        MessageBoxW(g_hwndMain, msg, lang->msgError, MB_OK | MB_ICONERROR);
        return;
    }
    g_currentMode = 4;
    mf_render_next_frame();
    swprintf(msg, 512, L"Media Foundation + D3D11 [%ls]: %ls", mf_get_renderer_name(), mf_get_decoder_info());
    UpdateStatus(L"%ls", msg);
    SetTimer(g_hwndMain, TIMER_RENDER, 33, NULL);
    g_renderTimerActive = 1;
    EnableButtons(TRUE);
    g_switching = 0;
}

static void StartMFD3D12(void)
{
    const LangStrings *lang = Lang_GetStrings();
    wchar_t msg[512];
    if (g_switching) return;
    if (!CheckFileSelected()) return;
    if (!CheckFileExists()) return;
    g_switching = 1;
    EnableButtons(FALSE);
    StopAll();
    UpdateStatus(L"MF + D3D12: %ls", lang->statusOpening);
    int ret = mf_open(g_filePath, g_hwndDisplay, 3);
    if (ret != 0) {
        swprintf(msg, 512, L"MF+D3D12: %ls - %ls", lang->msgCannotPlay, g_filePath);
        UpdateStatus(L"%ls", msg);
        EnableButtons(TRUE);
        g_switching = 0;
        MessageBoxW(g_hwndMain, msg, lang->msgError, MB_OK | MB_ICONERROR);
        return;
    }
    g_currentMode = 6;
    mf_render_next_frame();
    swprintf(msg, 512, L"MF + D3D12 [%ls]: %ls", mf_get_renderer_name(), mf_get_decoder_info());
    UpdateStatus(L"%ls", msg);
    SetTimer(g_hwndMain, TIMER_RENDER, 33, NULL);
    g_renderTimerActive = 1;
    EnableButtons(TRUE);
    g_switching = 0;
}

static int OpenFileDialog(HWND hwnd, wchar_t *path, int path_len)
{
    const LangStrings *lang = Lang_GetStrings();
    OPENFILENAMEW ofn;
    wchar_t filter[512];
    int pos = 0;
    
    /* Build filter string in required format: "Display\0Pattern\0Display\0Pattern\0\0" */
    
    /* Video files */
    pos += swprintf(filter + pos, 512 - pos, L"%ls (*.mp4;*.avi;*.mkv;*.wmv;*.mov;*.flv;*.webm;*.m4v)", lang->fileTypeVideo) + 1;
    pos += swprintf(filter + pos, 512 - pos, L"*.mp4;*.avi;*.mkv;*.wmv;*.mov;*.flv;*.webm;*.m4v") + 1;
    
    /* Audio files */
    pos += swprintf(filter + pos, 512 - pos, L"%ls (*.mp3;*.wav;*.aac;*.flac;*.ogg;*.wma;*.m4a)", lang->fileTypeAudio) + 1;
    pos += swprintf(filter + pos, 512 - pos, L"*.mp3;*.wav;*.aac;*.flac;*.ogg;*.wma;*.m4a") + 1;
    
    /* All files */
    pos += swprintf(filter + pos, 512 - pos, L"%ls (*.*)", lang->fileTypeAll) + 1;
    pos += swprintf(filter + pos, 512 - pos, L"*.*") + 1;
    
    /* Double null terminator */
    filter[pos] = L'\0';
    
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize  = sizeof(ofn);
    ofn.hwndOwner    = hwnd;
    ofn.lpstrFilter  = filter;
    ofn.lpstrFile    = path;
    ofn.nMaxFile     = path_len;
    ofn.lpstrTitle   = lang->fileDialogTitle;
    ofn.lpstrInitialDir = g_config.lastOpenDir;
    ofn.Flags        = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    return GetOpenFileNameW(&ofn);
}

/* About dialog procedure */
static INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG: {
        const LangStrings *lang = Lang_GetStrings();
        SetWindowTextW(hDlg, lang->aboutTitle);
        SetDlgItemTextW(hDlg, IDC_ABOUT_VERSION, lang->aboutVersion);
        SetDlgItemTextW(hDlg, IDOK, lang->btnOk);

        /* Set compiler info text */
        wchar_t compilerInfo[256];
        swprintf(compilerInfo, 256, COMPILER_INFO_FMT, COMPILER_INFO_ARGS);
        SetDlgItemTextW(hDlg, IDC_ABOUT_COMPILER, compilerInfo);
        return TRUE;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        }
        break;

    case WM_NOTIFY: {
        NMHDR *nmhdr = (NMHDR *)lParam;
        if (nmhdr->idFrom == IDC_ABOUT_LINK && nmhdr->code == NM_CLICK) {
            NMLINK *nmlink = (NMLINK *)lParam;
            ShellExecuteW(NULL, L"open", nmlink->item.szUrl, NULL, NULL, SW_SHOWNORMAL);
            return TRUE;
        }
        break;
    }
    }
    return FALSE;
}

/* Show about dialog */
static void ShowAboutDialog(HWND hwndParent)
{
    DialogBoxW(NULL, MAKEINTRESOURCEW(IDD_ABOUT), hwndParent, AboutDlgProc);
}

/* Settings dialog procedure */
static INT_PTR CALLBACK SettingsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG: {
        const LangStrings *lang = Lang_GetStrings();
        SetWindowTextW(hDlg, lang->settingsTitle);
        SetDlgItemTextW(hDlg, IDC_LANG_LABEL, lang->langSettingsLabel);
        SetDlgItemTextW(hDlg, IDC_LANG_HINT, lang->langSettingsHint);
        SetDlgItemTextW(hDlg, IDC_WINE_FIX_LABEL, lang->wineFixLabel);
        SetDlgItemTextW(hDlg, IDOK, lang->btnOk);
        SetDlgItemTextW(hDlg, IDCANCEL, lang->btnCancel);

        /* Populate language combo */
        HWND hCombo = GetDlgItem(hDlg, IDC_LANG_COMBO);
        SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)lang->langChinese);
        SendMessageW(hCombo, CB_ADDSTRING, 0, (LPARAM)lang->langEnglish);

        /* Select current language */
        int currentLang = Lang_GetCurrent();
        SendMessageW(hCombo, CB_SETCURSEL, (WPARAM)currentLang, 0);

        /* Set wine fix checkbox */
        CheckDlgButton(hDlg, IDC_WINE_FIX_CHECK, g_config.wine_fix ? BST_CHECKED : BST_UNCHECKED);
        return TRUE;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            /* Get language setting */
            HWND hCombo = GetDlgItem(hDlg, IDC_LANG_COMBO);
            int selected = (int)SendMessageW(hCombo, CB_GETCURSEL, 0, 0);
            int langChanged = (selected != CB_ERR && selected != Lang_GetCurrent());

            /* Get wine fix setting */
            int wineFix = (IsDlgButtonChecked(hDlg, IDC_WINE_FIX_CHECK) == BST_CHECKED) ? 1 : 0;

            /* Save settings */
            if (selected != CB_ERR) {
                g_config.language = selected;
            }
            g_config.wine_fix = wineFix;
            Config_Save(&g_config);
            ds_set_wine_fix(wineFix);

            /* Show restart message if language changed */
            if (langChanged) {
                const LangStrings *lang = Lang_GetStrings();
                MessageBoxW(hDlg, lang->langSettingsHint, lang->settingsTitle,
                    MB_OK | MB_ICONINFORMATION);
            }
            EndDialog(hDlg, IDOK);
            return TRUE;
        }
        if (LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

/* Show settings dialog */
static void ShowSettingsDialog(HWND hwndParent)
{
    DialogBoxW(NULL, MAKEINTRESOURCEW(IDD_SETTINGS), hwndParent, SettingsDlgProc);
}

/* Window procedure */
static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE:
        CreateControls(hwnd);
        return 0;

    case WM_SIZE:
        ResizeControls(hwnd);
        return 0;

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDC_BTN_DIRECTSHOW:  StartDirectShow();    break;
        case IDC_BTN_DS_DXVA2:    StartDirectShowDXVA2(); break;
        case IDC_BTN_MF_SOFTWARE: StartMFSoftware();     break;
        case IDC_BTN_MF_DXVA2:    StartMFDXVA2();        break;
        case IDC_BTN_MF_D3D11:    StartMFD3D11();        break;
        case IDC_BTN_MF_D3D12:    StartMFD3D12();        break;
        case IDC_BTN_STOP:        StopAll();             break;
        case IDC_BTN_OPEN_FILE: {
            const LangStrings *lang = Lang_GetStrings();
            if (OpenFileDialog(hwnd, g_filePath, MAX_PATH)) {
                /* Save directory for next time */
                wcscpy_s(g_config.lastOpenDir, MAX_PATH, g_filePath);
                PathRemoveFileSpecW(g_config.lastOpenDir);
                Config_Save(&g_config);

                UpdateStatus(lang->statusFileSelected, g_filePath);
            }
            break;
        }
        case IDC_BTN_ABOUT:
            ShowAboutDialog(hwnd);
            break;
        case IDC_BTN_SETTINGS:
            ShowSettingsDialog(hwnd);
            break;
        case IDC_BTN_CLEAR_LOG:
            SetWindowTextW(g_hwndLog, L"");
            break;
        case IDC_BTN_EXPORT_LOG: {
            OPENFILENAMEW ofn;
            wchar_t savePath[MAX_PATH] = L"decoder_log.txt";
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize  = sizeof(ofn);
            ofn.hwndOwner    = hwnd;
            ofn.lpstrFilter  = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
            ofn.lpstrFile    = savePath;
            ofn.nMaxFile     = MAX_PATH;
            ofn.lpstrTitle   = L"导出日志";
            ofn.Flags        = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
            ofn.lpstrDefExt  = L"txt";
            if (GetSaveFileNameW(&ofn)) {
                HANDLE hFile = CreateFileW(savePath, GENERIC_WRITE, 0, NULL,
                    CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
                if (hFile != INVALID_HANDLE_VALUE) {
                    int len = GetWindowTextLengthW(g_hwndLog);
                    if (len > 0) {
                        wchar_t *buf = (wchar_t *)malloc((len + 1) * sizeof(wchar_t));
                        if (buf) {
                            GetWindowTextW(g_hwndLog, buf, len + 1);
                            /* Write UTF-8 BOM */
                            DWORD written;
                            BYTE bom[] = {0xEF, 0xBB, 0xBF};
                            WriteFile(hFile, bom, 3, &written, NULL);
                            /* Convert to UTF-8 and write */
                            int utf8Len = WideCharToMultiByte(CP_UTF8, 0, buf, len, NULL, 0, NULL, NULL);
                            char *utf8Buf = (char *)malloc(utf8Len + 1);
                            if (utf8Buf) {
                                WideCharToMultiByte(CP_UTF8, 0, buf, len, utf8Buf, utf8Len, NULL, NULL);
                                WriteFile(hFile, utf8Buf, utf8Len, &written, NULL);
                                free(utf8Buf);
                            }
                            free(buf);
                        }
                    }
                    CloseHandle(hFile);
                    UpdateStatus(L"日志已导出: %ls", savePath);
                }
            }
            break;
        }
        }
        return 0;

    case WM_TIMER:
        if (wParam == TIMER_RENDER) {
            const LangStrings *lang = Lang_GetStrings();
            if (g_currentMode == 1 || g_currentMode == 5) {
                /* DirectShow modes */
                if (!ds_is_playing()) {
                    StopAll();
                    UpdateStatus(L"DirectShow: %ls", lang->statusPlaybackComplete);
                } else {
                    double pos = ds_get_position();
                    double dur = ds_get_duration();
                    int hasVid = ds_has_video();
                    wchar_t statusMsg[512];
                    if (hasVid) {
                        swprintf(statusMsg, 512, L"DirectShow%s [%ls] | %dx%d | %02d:%02d / %02d:%02d",
                                 g_currentMode == 5 ? L" + DXVA2" : L"",
                                 ds_get_renderer_name(),
                                 ds_get_video_width(), ds_get_video_height(),
                                 (int)pos / 60, (int)pos % 60,
                                 (int)dur / 60, (int)dur % 60);
                    } else {
                        swprintf(statusMsg, 512, L"DirectShow%s [%ls] | %ls | %02d:%02d / %02d:%02d",
                                 g_currentMode == 5 ? L" + DXVA2" : L"",
                                 ds_get_renderer_name(),
                                 lang->statusPureAudio,
                                 (int)pos / 60, (int)pos % 60,
                                 (int)dur / 60, (int)dur % 60);
                    }
                    UpdateStatus(statusMsg);
                }
            } else if (g_currentMode == 2 || g_currentMode == 3 || g_currentMode == 4 || g_currentMode == 6) {
                /* Media Foundation modes */
                int ret = mf_render_next_frame();
                if (ret == 1) {
                    StopAll();
                    UpdateStatus(L"Media Foundation: %ls", lang->statusPlaybackComplete);
                } else if (ret == -1 && mf_get_width() > 0) {
                    StopAll();
                    UpdateStatus(L"Media Foundation: %ls", lang->statusDecodeError);
                } else {
                    /* Update status with media info and progress */
                    double pos = (double)mf_get_position() / 10000000.0;
                    double dur = (double)mf_get_duration() / 10000000.0;
                    int hasVid = mf_has_video();
                    int hasAud = mf_has_audio();
                    int isHW = mf_is_using_dxva2();
                    wchar_t statusMsg[512];
                    const wchar_t *hwTag = lang->hwSoftware;
                    if (g_currentMode == 3) hwTag = isHW ? lang->hwDxva2 : lang->hwSoftware;
                    else if (g_currentMode == 4) hwTag = isHW ? lang->hwD3d11 : lang->hwSoftware;
                    else if (g_currentMode == 6) hwTag = lang->hwD3d12;

                    if (hasVid) {
                        double fps = mf_get_video_fps();
                        UINT32 vbr = mf_get_video_bitrate();
                        UINT32 abr = mf_get_audio_bitrate();
                        wchar_t vidPart[128] = {0};
                        if (fps > 0 && vbr > 0)
                            swprintf(vidPart, 128, L"%ls %dx%d %.1ffps %ukbps", mf_get_video_codec(), mf_get_width(), mf_get_height(), fps, vbr / 1000);
                        else if (fps > 0)
                            swprintf(vidPart, 128, L"%ls %dx%d %.1ffps", mf_get_video_codec(), mf_get_width(), mf_get_height(), fps);
                        else
                            swprintf(vidPart, 128, L"%ls %dx%d", mf_get_video_codec(), mf_get_width(), mf_get_height());
                        wchar_t audPart[64] = {0};
                        if (hasAud && abr > 0)
                            swprintf(audPart, 64, L"%ls %ukbps", mf_get_audio_codec(), abr / 1000);
                        else if (hasAud)
                            swprintf(audPart, 64, L"%ls", mf_get_audio_codec());
                        if (audPart[0])
                            swprintf(statusMsg, 512, L"MF [%ls] %ls | %ls | %ls | %02d:%02d/%02d:%02d drop %d/%d",
                                     mf_get_renderer_name(), hwTag, vidPart, audPart,
                                     (int)pos / 60, (int)pos % 60, (int)dur / 60, (int)dur % 60,
                                     mf_get_dropped_frames(), mf_get_total_frames());
                        else
                            swprintf(statusMsg, 512, L"MF [%ls] %ls | %ls | %02d:%02d/%02d:%02d drop %d/%d",
                                     mf_get_renderer_name(), hwTag, vidPart,
                                     (int)pos / 60, (int)pos % 60, (int)dur / 60, (int)dur % 60,
                                     mf_get_dropped_frames(), mf_get_total_frames());
                    } else if (hasAud) {
                        UINT32 abr = mf_get_audio_bitrate();
                        if (abr > 0) {
                            swprintf(statusMsg, 512, L"MF [%ls] %ls | %ls %ls %ukbps | %02d:%02d/%02d:%02d",
                                     mf_get_renderer_name(), hwTag, lang->statusPureAudio, mf_get_audio_codec(), abr / 1000,
                                     (int)pos / 60, (int)pos % 60, (int)dur / 60, (int)dur % 60);
                        } else {
                            swprintf(statusMsg, 512, L"MF [%ls] %ls | %ls %ls | %02d:%02d/%02d:%02d",
                                     mf_get_renderer_name(), hwTag, lang->statusPureAudio, mf_get_audio_codec(),
                                     (int)pos / 60, (int)pos % 60, (int)dur / 60, (int)dur % 60);
                        }
                    } else {
                        swprintf(statusMsg, 512, L"MF [%ls] %ls | %02d:%02d/%02d:%02d",
                                 mf_get_renderer_name(), hwTag,
                                 (int)pos / 60, (int)pos % 60, (int)dur / 60, (int)dur % 60);
                    }
                    UpdateStatus(statusMsg);
                }
            }
        }
        return 0;

    case WM_ERASEBKGND: {
        HDC hdc = (HDC)wParam;
        RECT rc;
        GetClientRect(hwnd, &rc);
        FillRect(hdc, &rc, (HBRUSH)(COLOR_BTNFACE + 1));
        return 1;
    }

    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        HWND hctl = (HWND)lParam;
        if (hctl == g_hwndDisplay) {
            SetBkColor(hdc, RGB(0, 0, 0));
            SetTextColor(hdc, RGB(255, 255, 255));
            if (!g_hBrushBlack) g_hBrushBlack = CreateSolidBrush(RGB(0, 0, 0));
            return (LRESULT)g_hBrushBlack;
        }
        break;
    }

    case WM_GETMINMAXINFO: {
        MINMAXINFO *mmi = (MINMAXINFO *)lParam;
        mmi->ptMinTrackSize.x = 640;
        mmi->ptMinTrackSize.y = 480;
        return 0;
    }

    case WM_DESTROY:
        StopAll();
        if (g_hFont) DeleteObject(g_hFont);
        if (g_hBrushBlack) DeleteObject(g_hBrushBlack);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}