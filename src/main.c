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

/* Button layout constants */
#define BTN_H       30
#define BTN_GAP     6
#define BTN_PAD     8
#define ROW1_Y      BTN_PAD
#define ROW2_Y      (BTN_PAD + BTN_H + BTN_GAP)
#define TOOLBAR_H   (BTN_PAD + BTN_H + BTN_GAP + BTN_H + BTN_PAD)

/* Global controls */
static HWND g_hwndMain       = NULL;
static HWND g_hwndBtnDS      = NULL;
static HWND g_hwndBtnDSDxva2 = NULL;
static HWND g_hwndBtnMF      = NULL;
static HWND g_hwndBtnDxva2   = NULL;
static HWND g_hwndBtnD3D11   = NULL;
static HWND g_hwndBtnStop    = NULL;
static HWND g_hwndBtnOpen    = NULL;
static HWND g_hwndDisplay    = NULL;
static HWND g_hwndStatus     = NULL;
static HFONT g_hFont         = NULL;

/* Current state */
static int   g_currentMode   = 0;
static int   g_renderTimerActive = 0;
static wchar_t g_filePath[MAX_PATH] = {0};
static HBRUSH g_hBrushBlack  = NULL;

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

/* Helper: create a button */
static HWND MakeButton(HWND parent, const wchar_t *text, int id, int x, int y, int w)
{
    HWND h = CreateWindowW(L"BUTTON", text,
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT,
        x, y, w, BTN_H, parent, (HMENU)(UINT_PTR)id, NULL, NULL);
    if (h && g_hFont) SendMessageW(h, WM_SETFONT, (WPARAM)g_hFont, TRUE);
    return h;
}

/* Entry point */
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPWSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEXW wc = {0};
    MSG msg;
    HRESULT hr;

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr)) {
        MessageBoxW(NULL, L"COM initialization failed", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(icex);
    icex.dwICC  = ICC_BAR_CLASSES | ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

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

    g_hwndMain = CreateWindowExW(
        0, CLASS_NAME, WINDOW_TITLE,
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

    /* Default file path */
    {
        wchar_t exe_dir[MAX_PATH];
        GetModuleFileNameW(NULL, exe_dir, MAX_PATH);
        wchar_t *last_slash = wcsrchr(exe_dir, L'\\');
        wchar_t *last_fwd_slash = wcsrchr(exe_dir, L'/');
        if (last_fwd_slash && (!last_slash || last_fwd_slash > last_slash))
            last_slash = last_fwd_slash;
        if (last_slash) {
            *(last_slash + 1) = L'\0';
            swprintf(g_filePath, MAX_PATH, L"%s%s", exe_dir, DEFAULT_FILE);
        } else {
            wcscpy(g_filePath, DEFAULT_FILE);
        }
        DWORD fileAttr = GetFileAttributesW(g_filePath);
        if (fileAttr == INVALID_FILE_ATTRIBUTES) {
            wchar_t cwd[MAX_PATH];
            GetCurrentDirectoryW(MAX_PATH, cwd);
            swprintf(g_filePath, MAX_PATH, L"%s\\%s", cwd, DEFAULT_FILE);
            fileAttr = GetFileAttributesW(g_filePath);
            if (fileAttr == INVALID_FILE_ATTRIBUTES)
                wcscpy(g_filePath, DEFAULT_FILE);
        }
    }

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
    /* Common font */
    g_hFont = CreateFontW(-13, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                          CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

    /* Row 1: decode methods */
    int bw = 140;
    int x = BTN_PAD;
    g_hwndBtnDS      = MakeButton(hwnd, L"DirectShow 播放",    IDC_BTN_DIRECTSHOW,  x, ROW1_Y, bw);
    x += bw + BTN_GAP;
    g_hwndBtnDSDxva2 = MakeButton(hwnd, L"DS + DXVA2",         IDC_BTN_DS_DXVA2,    x, ROW1_Y, bw);
    x += bw + BTN_GAP;
    g_hwndBtnMF      = MakeButton(hwnd, L"MF 软件解码",        IDC_BTN_MF_SOFTWARE, x, ROW1_Y, bw);
    x += bw + BTN_GAP;
    g_hwndBtnDxva2   = MakeButton(hwnd, L"MF + DXVA2",         IDC_BTN_MF_DXVA2,    x, ROW1_Y, bw);
    x += bw + BTN_GAP;
    g_hwndBtnD3D11   = MakeButton(hwnd, L"MF + D3D11",         IDC_BTN_MF_D3D11,    x, ROW1_Y, bw);

    /* Row 2: control */
    x = BTN_PAD;
    g_hwndBtnStop    = MakeButton(hwnd, L"停止播放",           IDC_BTN_STOP,        x, ROW2_Y, 100);
    x += 100 + BTN_GAP;
    g_hwndBtnOpen    = MakeButton(hwnd, L"打开文件...",        IDC_BTN_OPEN_FILE,   x, ROW2_Y, 120);

    /* Video display area */
    g_hwndDisplay = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"STATIC", L"",
        WS_CHILD | WS_VISIBLE | SS_SIMPLE,
        0, 0, 0, 0,
        hwnd, (HMENU)(UINT_PTR)IDC_VIDEO_DISPLAY, NULL, NULL);

    /* Status bar */
    g_hwndStatus = CreateWindowExW(
        0, STATUSCLASSNAMEW, L"就绪 - 选择解码方式开始播放",
        WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
        0, 0, 0, 0,
        hwnd, (HMENU)(UINT_PTR)IDC_STATUS_BAR, NULL, NULL);
}

/* Resize controls */
static void ResizeControls(HWND hwnd)
{
    RECT rc;
    GetClientRect(hwnd, &rc);

    /* Status bar */
    SendMessageW(g_hwndStatus, WM_SIZE, 0, 0);
    RECT rcStatus;
    GetWindowRect(g_hwndStatus, &rcStatus);
    int status_h = rcStatus.bottom - rcStatus.top;

    /* Video display fills the rest */
    int display_y = TOOLBAR_H;
    int display_h = rc.bottom - display_y - status_h;
    if (display_h < 0) display_h = 0;

    MoveWindow(g_hwndDisplay, 0, display_y, rc.right, display_h, TRUE);

    /* DirectShow resize */
    if (g_currentMode == 1 || g_currentMode == 5) {
        RECT vrc;
        GetClientRect(g_hwndDisplay, &vrc);
        ds_resize(0, 0, vrc.right, vrc.bottom);
    }
}

static void UpdateStatus(const wchar_t *msg)
{
    if (g_hwndStatus)
        SendMessageW(g_hwndStatus, SB_SETTEXTW, 0, (LPARAM)msg);
}

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
    if (g_hwndDisplay) InvalidateRect(g_hwndDisplay, NULL, TRUE);
    UpdateStatus(L"已停止");
}

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
    if (ret != 0) { UpdateStatus(L"DirectShow: 播放失败"); ds_stop(); return; }
    g_currentMode = 1;
    swprintf(msg, 512, L"DirectShow: 正在播放 %ls", g_filePath);
    UpdateStatus(msg);
    SetTimer(g_hwndMain, TIMER_RENDER, 100, NULL);
    g_renderTimerActive = 1;
}

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
    if (ret != 0) { UpdateStatus(L"DirectShow + DXVA2: 播放失败"); ds_stop(); return; }
    g_currentMode = 5;
    swprintf(msg, 512, L"DirectShow + DXVA2: 正在播放 %ls%s", g_filePath,
             ds_is_using_dxva2() ? L" (硬件加速)" : L" (软件回退)");
    UpdateStatus(msg);
    SetTimer(g_hwndMain, TIMER_RENDER, 100, NULL);
    g_renderTimerActive = 1;
}

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
    SetTimer(g_hwndMain, TIMER_RENDER, 33, NULL);
    g_renderTimerActive = 1;
}

static void StartMFDXVA2(void)
{
    wchar_t msg[512];
    StopAll();
    UpdateStatus(L"Media Foundation + DXVA2: 正在初始化...");
    if (!dxva2_check_support())
        UpdateStatus(L"DXVA2: 硬件加速不可用，将使用软件解码");
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
    SetTimer(g_hwndMain, TIMER_RENDER, 33, NULL);
    g_renderTimerActive = 1;
}

static void StartMFD3D11(void)
{
    wchar_t msg[512];
    StopAll();
    UpdateStatus(L"Media Foundation + D3D11: 正在初始化...");
    if (!d3d11_video_check_support())
        UpdateStatus(L"D3D11: 硬件加速不可用，将使用软件解码");
    if (d3d11_video_init(g_hwndDisplay, 1920, 1080) != 0)
        UpdateStatus(L"D3D11: 设备初始化失败，将使用软件解码");
    int ret = mf_open(g_filePath, g_hwndDisplay, 2);
    if (ret != 0) {
        swprintf(msg, 512, L"MF+D3D11: 打开失败 - %ls", g_filePath);
        UpdateStatus(msg);
        MessageBoxW(g_hwndMain, msg, L"错误", MB_OK | MB_ICONERROR);
        return;
    }
    g_currentMode = 4;
    swprintf(msg, 512, L"Media Foundation + D3D11: %ls", mf_get_decoder_info());
    UpdateStatus(msg);
    SetTimer(g_hwndMain, TIMER_RENDER, 33, NULL);
    g_renderTimerActive = 1;
}

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
        case IDC_BTN_DIRECTSHOW:  StartDirectShow();    break;
        case IDC_BTN_DS_DXVA2:    StartDirectShowDXVA2(); break;
        case IDC_BTN_MF_SOFTWARE: StartMFSoftware();     break;
        case IDC_BTN_MF_DXVA2:    StartMFDXVA2();        break;
        case IDC_BTN_MF_D3D11:    StartMFD3D11();        break;
        case IDC_BTN_STOP:        StopAll();             break;
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
                if (!ds_is_playing()) {
                    StopAll();
                    UpdateStatus(g_currentMode == 5 ?
                        L"DirectShow + DXVA2: 播放完成" : L"DirectShow: 播放完成");
                } else {
                    wchar_t msg[256];
                    swprintf(msg, 256, L"%ls: %.1f / %.1f 秒",
                             g_currentMode == 5 ? L"DirectShow + DXVA2" : L"DirectShow",
                             ds_get_position(), ds_get_duration());
                    UpdateStatus(msg);
                }
            } else if (g_currentMode == 2 || g_currentMode == 3 || g_currentMode == 4) {
                int ret = mf_render_next_frame();
                if (ret == 1)      { StopAll(); UpdateStatus(L"Media Foundation: 播放完成"); }
                else if (ret == -1) { StopAll(); UpdateStatus(L"Media Foundation: 解码错误"); }
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
