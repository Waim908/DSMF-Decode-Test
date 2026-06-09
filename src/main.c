/*
 * Windows Video Decoder Test
 * Main GUI - Win32 window with buttons for different decoding methods.
 */
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <string.h>

#include "resource.h"
#include "directshow_decoder.h"
#include "mf_decoder.h"
#include "dxva2_helper.h"
#include "d3d11_video_helper.h"

/* Window class names */
static const wchar_t CLASS_NAME[] = L"VideoDecoderTestClass";
static const wchar_t WINDOW_TITLE[] = L"DirectShowMediaFoundationDecodeTest";

/* Global controls */
static HWND g_hwndMain       = NULL;
static HWND g_hwndDisplay    = NULL;
static HWND g_hwndBtnDS      = NULL;
static HWND g_hwndBtnDSDxva2 = NULL;
static HWND g_hwndBtnMF      = NULL;
static HWND g_hwndBtnDxva2   = NULL;
static HWND g_hwndBtnD3D11   = NULL;
static HWND g_hwndBtnStop    = NULL;
static HWND g_hwndStatus     = NULL;
static HFONT g_hFont         = NULL;

/* Current state */
static int   g_currentMode   = 0;  /* 0=none, 1=DS, 2=MF, 3=MF+DXVA2, 4=MF+D3D11, 5=DS+DXVA2 */
static int   g_renderTimerActive = 0;
static wchar_t g_filePath[MAX_PATH] = {0};

/* Default file path */
static const wchar_t DEFAULT_FILE[] = L"demo.mp4";

/* Forward declarations */
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static void CreateControls(HWND hwnd);
static void UpdateStatus(const wchar_t *msg);
static void StartDirectShow(void);
static void StartDirectShowDXVA2(void);
static void StartMFSoftware(void);
static void StartMFDXVA2(void);
static void StartMFD3D11(void);
static void StopAll(void);
static void ResizeControls(HWND hwnd);
static int  OpenFileDialog(HWND hwnd, wchar_t *path, int path_len);

/* Entry point */
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPWSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEXW wc = {0};
    MSG msg;
    HRESULT hr;

    /* Initialize COM */
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr)) {
        MessageBoxW(NULL, L"COM initialization failed", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    /* Initialize common controls */
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(icex);
    icex.dwICC  = ICC_BAR_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    /* Register window class */
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WndProc;
    wc.hInstance      = hInstance;
    wc.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground  = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName  = CLASS_NAME;
    wc.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm        = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassExW(&wc);

    /* Create main window */
    g_hwndMain = CreateWindowExW(
        0, CLASS_NAME, WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 960, 640,
        NULL, NULL, hInstance, NULL);

    if (!g_hwndMain) {
        MessageBoxW(NULL, L"Window creation failed", L"Error", MB_OK | MB_ICONERROR);
        CoUninitialize();
        return 1;
    }

    ShowWindow(g_hwndMain, nCmdShow);
    UpdateWindow(g_hwndMain);

    /* Default file path - get executable directory */
    {
        wchar_t exe_dir[MAX_PATH];
        GetModuleFileNameW(NULL, exe_dir, MAX_PATH);
        /* Trim to directory - support both \\ and / separators */
        wchar_t *last_slash = wcsrchr(exe_dir, L'\\');
        wchar_t *last_fwd_slash = wcsrchr(exe_dir, L'/');
        if (last_fwd_slash && (!last_slash || last_fwd_slash > last_slash)) {
            last_slash = last_fwd_slash;
        }
        if (last_slash) {
            *(last_slash + 1) = L'\0';
            swprintf(g_filePath, MAX_PATH, L"%s%s", exe_dir, DEFAULT_FILE);
        } else {
            wcscpy(g_filePath, DEFAULT_FILE);
        }
        
        /* Check if file exists, if not try current directory */
        DWORD fileAttr = GetFileAttributesW(g_filePath);
        if (fileAttr == INVALID_FILE_ATTRIBUTES) {
            /* Try current working directory */
            wchar_t cwd[MAX_PATH];
            GetCurrentDirectoryW(MAX_PATH, cwd);
            swprintf(g_filePath, MAX_PATH, L"%s\\%s", cwd, DEFAULT_FILE);
            
            /* If still not found, use just the filename */
            fileAttr = GetFileAttributesW(g_filePath);
            if (fileAttr == INVALID_FILE_ATTRIBUTES) {
                wcscpy(g_filePath, DEFAULT_FILE);
            }
        }
    }

    /* Message loop */
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
    int btn_y = 8;
    int btn_h = 32;
    int btn_w = 180;
    int btn_gap = 8;
    int x = 8;

    /* Button font */
    g_hFont = CreateFontW(-14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                          CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

    /* Buttons */
    g_hwndBtnDS = CreateWindowW(L"BUTTON", L"DirectShow 播放",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x, btn_y, btn_w, btn_h, hwnd, (HMENU)(UINT_PTR)IDC_BTN_DIRECTSHOW,
        NULL, NULL);
    x += btn_w + btn_gap;

    g_hwndBtnDSDxva2 = CreateWindowW(L"BUTTON", L"DS + DXVA2 硬件加速",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x, btn_y, btn_w + 20, btn_h, hwnd, (HMENU)(UINT_PTR)IDC_BTN_DS_DXVA2,
        NULL, NULL);
    x += btn_w + 20 + btn_gap;

    g_hwndBtnMF = CreateWindowW(L"BUTTON", L"MF 软件解码",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x, btn_y, btn_w, btn_h, hwnd, (HMENU)(UINT_PTR)IDC_BTN_MF_SOFTWARE,
        NULL, NULL);
    x += btn_w + btn_gap;

    g_hwndBtnDxva2 = CreateWindowW(L"BUTTON", L"MF + DXVA2 硬件加速",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x, btn_y, btn_w + 20, btn_h, hwnd, (HMENU)(UINT_PTR)IDC_BTN_MF_DXVA2,
        NULL, NULL);
    x += btn_w + 20 + btn_gap;

    g_hwndBtnD3D11 = CreateWindowW(L"BUTTON", L"MF + D3D11 硬件加速",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x, btn_y, btn_w + 20, btn_h, hwnd, (HMENU)(UINT_PTR)IDC_BTN_MF_D3D11,
        NULL, NULL);
    x += btn_w + 20 + btn_gap;

    g_hwndBtnStop = CreateWindowW(L"BUTTON", L"停止",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x, btn_y, 80, btn_h, hwnd, (HMENU)(UINT_PTR)IDC_BTN_STOP,
        NULL, NULL);
    x += 80 + btn_gap;

    /* Open file button */
    CreateWindowW(L"BUTTON", L"打开文件...",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        x, btn_y, 100, btn_h, hwnd, (HMENU)(UINT_PTR)IDC_BTN_OPEN_FILE,
        NULL, NULL);

    /* Video display area */
    g_hwndDisplay = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | SS_BLACKRECT,
        0, 0, 0, 0,
        hwnd, (HMENU)(UINT_PTR)IDC_VIDEO_DISPLAY,
        NULL, NULL);

    /* Status bar */
    g_hwndStatus = CreateWindowExW(
        0, STATUSCLASSNAMEW, L"就绪 - 选择解码方式开始播放",
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0,
        hwnd, (HMENU)(UINT_PTR)IDC_STATUS_BAR,
        NULL, NULL);

    /* Apply font */
    if (g_hFont) {
        SendMessageW(g_hwndBtnDS,      WM_SETFONT, (WPARAM)g_hFont, TRUE);
        SendMessageW(g_hwndBtnDSDxva2, WM_SETFONT, (WPARAM)g_hFont, TRUE);
        SendMessageW(g_hwndBtnMF,      WM_SETFONT, (WPARAM)g_hFont, TRUE);
        SendMessageW(g_hwndBtnDxva2,   WM_SETFONT, (WPARAM)g_hFont, TRUE);
        SendMessageW(g_hwndBtnD3D11,   WM_SETFONT, (WPARAM)g_hFont, TRUE);
        SendMessageW(g_hwndBtnStop,    WM_SETFONT, (WPARAM)g_hFont, TRUE);
    }
}

/* Resize controls on window resize */
static void ResizeControls(HWND hwnd)
{
    RECT rc;
    GetClientRect(hwnd, &rc);

    int btn_area_h = 48;  /* Top button area height */
    int status_h   = 24;

    /* Status bar */
    SendMessageW(g_hwndStatus, WM_SIZE, 0, 0);

    /* Video display area */
    MoveWindow(g_hwndDisplay, 0, btn_area_h,
               rc.right, rc.bottom - btn_area_h - status_h, TRUE);

    /* Update DirectShow video window to match new display area */
    if (g_currentMode == 1) {
        RECT vrc;
        GetClientRect(g_hwndDisplay, &vrc);
        ds_resize(0, 0, vrc.right, vrc.bottom);
    }
}

/* Update status bar text */
static void UpdateStatus(const wchar_t *msg)
{
    if (g_hwndStatus) {
        SendMessageW(g_hwndStatus, SB_SETTEXTW, 0, (LPARAM)msg);
    }
}

/* Stop all decoders */
static void StopAll(void)
{
    if (g_renderTimerActive) {
        KillTimer(g_hwndMain, TIMER_RENDER);
        g_renderTimerActive = 0;
    }
    ds_stop();
    mf_stop();
    dxva2_cleanup();
    d3d11_video_cleanup();
    g_currentMode = 0;

    /* Clear display */
    if (g_hwndDisplay) {
        InvalidateRect(g_hwndDisplay, NULL, TRUE);
    }
    UpdateStatus(L"已停止");
}

/* Start DirectShow playback */
static void StartDirectShow(void)
{
    wchar_t msg[512];

    StopAll();

    UpdateStatus(L"DirectShow: 正在打开...");
    int ret = ds_open(g_filePath, g_hwndDisplay);
    if (ret != 0) {
        swprintf(msg, 512, L"DirectShow: 打开失败 - %ls", g_filePath);
        UpdateStatus(msg);
        MessageBoxW(g_hwndMain, msg, L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    ret = ds_play();
    if (ret != 0) {
        UpdateStatus(L"DirectShow: 播放失败");
        ds_stop();
        return;
    }

    g_currentMode = 1;
    swprintf(msg, 512, L"DirectShow: 正在播放 %ls", g_filePath);
    UpdateStatus(msg);

    /* Start timer to monitor playback status */
    SetTimer(g_hwndMain, TIMER_RENDER, 100, NULL);
    g_renderTimerActive = 1;
}

/* Start DirectShow with DXVA2 hardware acceleration */
static void StartDirectShowDXVA2(void)
{
    wchar_t msg[512];

    StopAll();

    UpdateStatus(L"DirectShow + DXVA2: 正在打开...");
    int ret = ds_open_dxva2(g_filePath, g_hwndDisplay, 1);
    if (ret != 0) {
        swprintf(msg, 512, L"DirectShow + DXVA2: 打开失败 - %ls", g_filePath);
        UpdateStatus(msg);
        MessageBoxW(g_hwndMain, msg, L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    ret = ds_play();
    if (ret != 0) {
        UpdateStatus(L"DirectShow + DXVA2: 播放失败");
        ds_stop();
        return;
    }

    g_currentMode = 5;
    if (ds_is_using_dxva2()) {
        swprintf(msg, 512, L"DirectShow + DXVA2: 正在播放 %ls (硬件加速)", g_filePath);
    } else {
        swprintf(msg, 512, L"DirectShow + DXVA2: 正在播放 %ls (软件回退)", g_filePath);
    }
    UpdateStatus(msg);

    /* Start timer to monitor playback status */
    SetTimer(g_hwndMain, TIMER_RENDER, 100, NULL);
    g_renderTimerActive = 1;
}

/* Start Media Foundation software decoding */
static void StartMFSoftware(void)
{
    wchar_t msg[512];

    StopAll();

    UpdateStatus(L"Media Foundation (软件): 正在打开...");
    int ret = mf_open(g_filePath, g_hwndDisplay, 0);
    if (ret != 0) {
        swprintf(msg, 512, L"Media Foundation: 打开失败 - %ls", g_filePath);
        UpdateStatus(msg);
        MessageBoxW(g_hwndMain, msg, L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    g_currentMode = 2;
    swprintf(msg, 512, L"Media Foundation (软件解码): %ls", mf_get_decoder_info());
    UpdateStatus(msg);

    /* Start timer for frame rendering */
    SetTimer(g_hwndMain, TIMER_RENDER, 33, NULL);  /* ~30fps */
    g_renderTimerActive = 1;
}

/* Start Media Foundation + DXVA2 hardware acceleration */
static void StartMFDXVA2(void)
{
    wchar_t msg[512];

    StopAll();

    UpdateStatus(L"Media Foundation + DXVA2: 正在初始化...");

    /* Check DXVA2 support */
    if (!dxva2_check_support()) {
        UpdateStatus(L"DXVA2: 硬件加速不可用，将使用软件解码");
    }

    int ret = mf_open(g_filePath, g_hwndDisplay, 1);
    if (ret != 0) {
        swprintf(msg, 512, L"MF+DXVA2: 打开失败 - %ls", g_filePath);
        UpdateStatus(msg);
        MessageBoxW(g_hwndMain, msg, L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    g_currentMode = 3;
    swprintf(msg, 512, L"Media Foundation + DXVA2: %ls", mf_get_decoder_info());
    UpdateStatus(msg);

    /* Start timer for frame rendering */
    SetTimer(g_hwndMain, TIMER_RENDER, 33, NULL);  /* ~30fps */
    g_renderTimerActive = 1;
}

/* Start Media Foundation + D3D11 hardware acceleration */
static void StartMFD3D11(void)
{
    wchar_t msg[512];

    StopAll();

    UpdateStatus(L"Media Foundation + D3D11: 正在初始化...");

    /* Check D3D11 support */
    if (!d3d11_video_check_support()) {
        UpdateStatus(L"D3D11: 硬件加速不可用，将使用软件解码");
    }

    /* Initialize D3D11 device */
    if (d3d11_video_init(g_hwndDisplay, 1920, 1080) != 0) {
        UpdateStatus(L"D3D11: 设备初始化失败，将使用软件解码");
    }

    int ret = mf_open(g_filePath, g_hwndDisplay, 2);  /* mode 2 = D3D11 */
    if (ret != 0) {
        swprintf(msg, 512, L"MF+D3D11: 打开失败 - %ls", g_filePath);
        UpdateStatus(msg);
        MessageBoxW(g_hwndMain, msg, L"错误", MB_OK | MB_ICONERROR);
        return;
    }

    g_currentMode = 4;
    swprintf(msg, 512, L"Media Foundation + D3D11: %ls", mf_get_decoder_info());
    UpdateStatus(msg);

    /* Start timer for frame rendering */
    SetTimer(g_hwndMain, TIMER_RENDER, 33, NULL);  /* ~30fps */
    g_renderTimerActive = 1;
}

/* Open file dialog */
static int OpenFileDialog(HWND hwnd, wchar_t *path, int path_len)
{
    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize  = sizeof(ofn);
    ofn.hwndOwner    = hwnd;
    ofn.lpstrFilter  = L"Video Files\0*.mp4;*.avi;*.mkv;*.wmv;*.mov;*.flv;*.webm;*.m4v\0"
                       L"All Files\0*.*\0";
    ofn.lpstrFile    = path;
    ofn.nMaxFile     = path_len;
    ofn.lpstrTitle   = L"选择视频文件";
    ofn.Flags        = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    return GetOpenFileNameW(&ofn);
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
        case IDC_BTN_DIRECTSHOW:
            StartDirectShow();
            break;
        case IDC_BTN_DS_DXVA2:
            StartDirectShowDXVA2();
            break;
        case IDC_BTN_MF_SOFTWARE:
            StartMFSoftware();
            break;
        case IDC_BTN_MF_DXVA2:
            StartMFDXVA2();
            break;
        case IDC_BTN_MF_D3D11:
            StartMFD3D11();
            break;
        case IDC_BTN_STOP:
            StopAll();
            break;
        case IDC_BTN_OPEN_FILE:
            if (OpenFileDialog(hwnd, g_filePath, MAX_PATH)) {
                wchar_t msg[512];
                swprintf(msg, 512, L"已选择文件: %ls", g_filePath);
                UpdateStatus(msg);
            }
            break;
        }
        return 0;

    case WM_TIMER:
        if (wParam == TIMER_RENDER) {
            if (g_currentMode == 1 || g_currentMode == 5) {
                /* DirectShow - check if still playing */
                if (!ds_is_playing()) {
                    StopAll();
                    if (g_currentMode == 5) {
                        UpdateStatus(L"DirectShow + DXVA2: 播放完成");
                    } else {
                        UpdateStatus(L"DirectShow: 播放完成");
                    }
                } else {
                    wchar_t msg[256];
                    if (g_currentMode == 5) {
                        swprintf(msg, 256, L"DirectShow + DXVA2: %.1f / %.1f 秒",
                                 ds_get_position(), ds_get_duration());
                    } else {
                        swprintf(msg, 256, L"DirectShow: %.1f / %.1f 秒",
                                 ds_get_position(), ds_get_duration());
                    }
                    UpdateStatus(msg);
                }
            } else if (g_currentMode == 2 || g_currentMode == 3 || g_currentMode == 4) {
                /* Media Foundation - render next frame */
                int ret = mf_render_next_frame();
                if (ret == 1) {
                    /* EOF */
                    StopAll();
                    UpdateStatus(L"Media Foundation: 播放完成");
                } else if (ret == -1) {
                    StopAll();
                    UpdateStatus(L"Media Foundation: 解码错误");
                }
            }
        }
        return 0;

    case WM_ERASEBKGND:
        /* Suppress background erase for video area */
        return 1;

    case WM_GETMINMAXINFO: {
        MINMAXINFO *mmi = (MINMAXINFO *)lParam;
        mmi->ptMinTrackSize.x = 640;
        mmi->ptMinTrackSize.y = 480;
        return 0;
    }

    case WM_DESTROY:
        StopAll();
        if (g_hFont) DeleteObject(g_hFont);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}
