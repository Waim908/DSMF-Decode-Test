/*
 * DirectShow Video Decoder
 * Uses IGraphBuilder + IMediaControl + IVideoWindow for playback.
 */
#include "directshow_decoder.h"
#include "log.h"

#include <dshow.h>
#include <stdio.h>
#include <shlwapi.h>

/* CLSID_EnhancedVideoRenderer: MSVC has extern in strmif.h, MinGW needs definition */
#ifndef _MSC_VER
static const CLSID CLSID_EnhancedVideoRenderer = {0xfa10746c, 0x9b63, 0x4b6c, {0xbc, 0x49, 0xfc, 0x30, 0x0e, 0xa5, 0xf2, 0x56}};
#endif

static IGraphBuilder    *pGraph    = NULL;
static IMediaControl    *pControl  = NULL;
static IMediaEvent      *pEvent    = NULL;
static IVideoWindow     *pVideo    = NULL;
static IBasicVideo      *pBasicVideo = NULL;
static IBasicAudio      *pAudio    = NULL;
static IMediaSeeking    *pSeeking  = NULL;
static int               g_playing = 0;
static int               g_dxva2   = 0;
static HWND              g_hwndDisplay = NULL;
static int               g_video_w = 0;
static int               g_video_h = 0;
static int               g_renderer_type = 0;  /* 0=Default, 1=VMR-9, 2=EVR */

static void ds_update_aspect(void)
{
    if (!pVideo || !g_hwndDisplay) return;
    RECT rc;
    GetClientRect(g_hwndDisplay, &rc);
    ds_resize(0, 0, rc.right, rc.bottom);
}

static void ds_cleanup(void)
{
    /* Hide video window and detach from owner */
    if (pVideo) {
        IVideoWindow_put_Visible(pVideo, OAFALSE);
        IVideoWindow_put_AutoShow(pVideo, OAFALSE);
        IVideoWindow_put_Owner(pVideo, 0);
    }

    /* Force-hide any remaining child windows in the display area (VMR-9/EVR) */
    if (g_hwndDisplay) {
        HWND child = GetWindow(g_hwndDisplay, GW_CHILD);
        while (child) {
            ShowWindow(child, SW_HIDE);
            child = GetWindow(child, GW_HWNDNEXT);
        }

        /* Clear display area to prevent residual frames */
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

    /* Release all DirectShow interfaces */
    if (pVideo)      { IVideoWindow_Release(pVideo); pVideo = NULL; }
    if (pBasicVideo)  { IBasicVideo_Release(pBasicVideo); pBasicVideo = NULL; }
    if (pSeeking)     { IMediaSeeking_Release(pSeeking); pSeeking = NULL; }
    if (pAudio)       { IBasicAudio_Release(pAudio); pAudio = NULL; }
    if (pEvent)       { IMediaEvent_Release(pEvent); pEvent = NULL; }
    if (pControl)     { IMediaControl_Release(pControl); pControl = NULL; }
    if (pGraph)       { IGraphBuilder_Release(pGraph); pGraph = NULL; }
    g_hwndDisplay = NULL;
    g_video_w = 0;
    g_video_h = 0;
    g_playing = 0;
    g_dxva2 = 0;
    g_renderer_type = 0;
}

int ds_open(const wchar_t *filepath, HWND hwnd_display)
{
    HRESULT hr;
    IBaseFilter *pSource = NULL;

    ds_cleanup();

    /* Create the Filter Graph Manager */
    hr = CoCreateInstance(&CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
                          &IID_IGraphBuilder, (void **)&pGraph);
    if (FAILED(hr)) { Log_Printf(L"DirectShow: Failed to create FilterGraph: 0x%08l", hr); return -1; }

    /* Query interfaces */
    hr = IGraphBuilder_QueryInterface(pGraph, &IID_IMediaControl, (void **)&pControl);
    if (FAILED(hr)) Log_Printf(L"DirectShow: Failed to query IMediaControl: 0x%08l", hr);
    hr = IGraphBuilder_QueryInterface(pGraph, &IID_IMediaEvent,   (void **)&pEvent);
    if (FAILED(hr)) Log_Printf(L"DirectShow: Failed to query IMediaEvent: 0x%08l", hr);
    hr = IGraphBuilder_QueryInterface(pGraph, &IID_IVideoWindow,  (void **)&pVideo);
    if (FAILED(hr)) Log_Printf(L"DirectShow: Failed to query IVideoWindow: 0x%08l", hr);
    hr = IGraphBuilder_QueryInterface(pGraph, &IID_IBasicVideo,   (void **)&pBasicVideo);
    if (FAILED(hr)) Log_Printf(L"DirectShow: Failed to query IBasicVideo: 0x%08l", hr);
    hr = IGraphBuilder_QueryInterface(pGraph, &IID_IBasicAudio,   (void **)&pAudio);
    if (FAILED(hr)) Log_Printf(L"DirectShow: Failed to query IBasicAudio: 0x%08l", hr);
    hr = IGraphBuilder_QueryInterface(pGraph, &IID_IMediaSeeking, (void **)&pSeeking);
    if (FAILED(hr)) Log_Printf(L"DirectShow: Failed to query IMediaSeeking: 0x%08l", hr);

    if (!pControl || !pVideo) {
        Log_Printf(L"DirectShow: Failed to query required interface");
        ds_cleanup();
        return -1;
    }

    /* Add source filter for the file */
    hr = IGraphBuilder_AddSourceFilter(pGraph, filepath, L"Source", &pSource);
    if (FAILED(hr)) {
        Log_Printf(L"DirectShow: Failed to add source filter for '%ls': 0x%08l", filepath, hr);
        ds_cleanup();
        return -1;
    }

    /* Render all output pins (auto-connects decoder + renderer) */
    hr = IGraphBuilder_RenderFile(pGraph, filepath, NULL);
    if (FAILED(hr)) {
        Log_Printf(L"DirectShow: RenderFile failed: 0x%08l", hr);
        if (pSource) IBaseFilter_Release(pSource);
        ds_cleanup();
        return -1;
    }

    if (pSource) IBaseFilter_Release(pSource);

    /* Embed video window into our display area */
    if (hwnd_display && pVideo) {
        g_hwndDisplay = hwnd_display;

        IVideoWindow_put_Owner(pVideo, (OAHWND)hwnd_display);
        IVideoWindow_put_WindowStyle(pVideo, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        IVideoWindow_put_MessageDrain(pVideo, (OAHWND)hwnd_display);
        IVideoWindow_put_AutoShow(pVideo, OAFALSE);

        /* Get native video size from IBasicVideo */
        g_video_w = 0;
        g_video_h = 0;
        if (pBasicVideo) {
            long vw = 0, vh = 0;
            IBasicVideo_get_SourceWidth(pBasicVideo, &vw);
            IBasicVideo_get_SourceHeight(pBasicVideo, &vh);
            if (vw > 0 && vh > 0) {
                g_video_w = vw;
                g_video_h = vh;
                Log_Printf(L"DirectShow: Native video size %ldx%ld", vw, vh);
            } else {
                Log_Printf(L"DirectShow: Failed to get video size from IBasicVideo (%ldx%ld)", vw, vh);
            }
        } else {
            Log_Printf(L"DirectShow: IBasicVideo not available");
        }

        /* Fit video to display area with aspect ratio */
        ds_update_aspect();
        IVideoWindow_put_Visible(pVideo, OATRUE);
        Log_Printf(L"DirectShow: Video window set to visible");
    }

    /* Show registered video decoders in log */
    ds_enum_filters(1);

    g_playing = 0;
    g_renderer_type = 0;  /* Default renderer */
    return 0;
}

int ds_open_dxva2(const wchar_t *filepath, HWND hwnd_display, int enable_dxva2)
{
    HRESULT hr;
    IBaseFilter *pSource = NULL;
    IBaseFilter *pRenderer = NULL;

    ds_cleanup();

    /* Create the Filter Graph Manager */
    hr = CoCreateInstance(&CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
                          &IID_IGraphBuilder, (void **)&pGraph);
    if (FAILED(hr)) { Log_Printf(L"DirectShow: Failed to create FilterGraph: 0x%08l", hr); return -1; }

    /* Query interfaces */
    hr = IGraphBuilder_QueryInterface(pGraph, &IID_IMediaControl, (void **)&pControl);
    if (FAILED(hr)) Log_Printf(L"DirectShow: Failed to query IMediaControl: 0x%08l", hr);
    hr = IGraphBuilder_QueryInterface(pGraph, &IID_IMediaEvent,   (void **)&pEvent);
    if (FAILED(hr)) Log_Printf(L"DirectShow: Failed to query IMediaEvent: 0x%08l", hr);
    hr = IGraphBuilder_QueryInterface(pGraph, &IID_IVideoWindow,  (void **)&pVideo);
    if (FAILED(hr)) Log_Printf(L"DirectShow: Failed to query IVideoWindow: 0x%08l", hr);
    hr = IGraphBuilder_QueryInterface(pGraph, &IID_IBasicVideo,   (void **)&pBasicVideo);
    if (FAILED(hr)) Log_Printf(L"DirectShow: Failed to query IBasicVideo: 0x%08l", hr);
    hr = IGraphBuilder_QueryInterface(pGraph, &IID_IBasicAudio,   (void **)&pAudio);
    if (FAILED(hr)) Log_Printf(L"DirectShow: Failed to query IBasicAudio: 0x%08l", hr);
    hr = IGraphBuilder_QueryInterface(pGraph, &IID_IMediaSeeking, (void **)&pSeeking);
    if (FAILED(hr)) Log_Printf(L"DirectShow: Failed to query IMediaSeeking: 0x%08l", hr);

    if (!pControl || !pVideo) {
        Log_Printf(L"DirectShow: Failed to query required interface");
        ds_cleanup();
        return -1;
    }

    /* Add source filter for the file */
    hr = IGraphBuilder_AddSourceFilter(pGraph, filepath, L"Source", &pSource);
    if (FAILED(hr)) {
        Log_Printf(L"DirectShow: Failed to add source filter for '%ls': 0x%08l", filepath, hr);
        ds_cleanup();
        return -1;
    }

    if (enable_dxva2) {
        /* Try to use VMR-9 first (supports DXVA2) */
        hr = CoCreateInstance(&CLSID_VideoMixingRenderer9, NULL, CLSCTX_INPROC_SERVER,
                              &IID_IBaseFilter, (void **)&pRenderer);
        if (SUCCEEDED(hr)) {
            hr = IGraphBuilder_AddFilter(pGraph, pRenderer, L"VMR-9");
            if (SUCCEEDED(hr)) {
#ifdef __MINGW32__
                Log_Printf(L"DirectShow: Using VMR-9 renderer (DXVA2 capable, MinGW build");
#else
                Log_Printf(L"DirectShow: Using VMR-9 renderer (DXVA2 capable");
#endif
                g_dxva2 = 1;
                g_renderer_type = 1;  /* VMR-9 */
            } else {
                Log_Printf(L"DirectShow: Failed to add VMR-9 filter: 0x%08l", hr);
                IBaseFilter_Release(pRenderer);
                pRenderer = NULL;
            }
        } else {
            Log_Printf(L"DirectShow: Failed to create VMR-9: 0x%08l", hr);
        }

        /* If VMR-9 failed, try EVR */
        if (!pRenderer) {
            hr = CoCreateInstance(&CLSID_EnhancedVideoRenderer, NULL, CLSCTX_INPROC_SERVER,
                                  &IID_IBaseFilter, (void **)&pRenderer);
            if (SUCCEEDED(hr)) {
                hr = IGraphBuilder_AddFilter(pGraph, pRenderer, L"EVR");
                if (SUCCEEDED(hr)) {
#ifdef __MINGW32__
                    Log_Printf(L"DirectShow: Using EVR renderer (DXVA2 capable, MinGW build");
#else
                    Log_Printf(L"DirectShow: Using EVR renderer (DXVA2 capable");
#endif
                    g_dxva2 = 1;
                    g_renderer_type = 2;  /* EVR */
                } else {
                    Log_Printf(L"DirectShow: Failed to add EVR filter: 0x%08l", hr);
                    IBaseFilter_Release(pRenderer);
                    pRenderer = NULL;
                }
            } else {
                Log_Printf(L"DirectShow: Failed to create EVR: 0x%08l", hr);
            }
        }
    }

    /* Render all output pins (auto-connects decoder + renderer) */
    hr = IGraphBuilder_RenderFile(pGraph, filepath, NULL);
    if (FAILED(hr)) {
        Log_Printf(L"DirectShow: RenderFile failed: 0x%08l", hr);
        if (pSource) IBaseFilter_Release(pSource);
        if (pRenderer) IBaseFilter_Release(pRenderer);
        ds_cleanup();
        return -1;
    }

    if (pSource) IBaseFilter_Release(pSource);
    if (pRenderer) IBaseFilter_Release(pRenderer);

    /* Embed video window into our display area */
    if (hwnd_display && pVideo) {
        g_hwndDisplay = hwnd_display;

        IVideoWindow_put_Owner(pVideo, (OAHWND)hwnd_display);
        IVideoWindow_put_WindowStyle(pVideo, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
        IVideoWindow_put_MessageDrain(pVideo, (OAHWND)hwnd_display);
        IVideoWindow_put_AutoShow(pVideo, OAFALSE);

        /* Get native video size from IBasicVideo */
        g_video_w = 0;
        g_video_h = 0;
        if (pBasicVideo) {
            long vw = 0, vh = 0;
            IBasicVideo_get_SourceWidth(pBasicVideo, &vw);
            IBasicVideo_get_SourceHeight(pBasicVideo, &vh);
            if (vw > 0 && vh > 0) {
                g_video_w = vw;
                g_video_h = vh;
                Log_Printf(L"DirectShow: Native video size %ldx%ld", vw, vh);
            } else {
                Log_Printf(L"DirectShow: Failed to get video size (%ldx%ld)", vw, vh);
            }
        }

        /* For VMR-9/EVR, set the video position directly */
        {
            RECT rc;
            GetClientRect(hwnd_display, &rc);
            int display_w = rc.right;
            int display_h = rc.bottom;

            if (g_video_w > 0 && g_video_h > 0) {
                /* Calculate aspect-ratio preserving size */
                float scale_x = (float)display_w / (float)g_video_w;
                float scale_y = (float)display_h / (float)g_video_h;
                float scale = (scale_x < scale_y) ? scale_x : scale_y;
                int draw_w = (int)(g_video_w * scale);
                int draw_h = (int)(g_video_h * scale);
                int draw_x = (display_w - draw_w) / 2;
                int draw_y = (display_h - draw_h) / 2;

                /* Set video position */
                IVideoWindow_SetWindowPosition(pVideo, draw_x, draw_y, draw_w, draw_h);
                Log_Printf(L"DirectShow: Video position set to %dx%d at (%d,%d", draw_w, draw_h, draw_x, draw_y);
            } else {
                /* No video size info, fill the window */
                IVideoWindow_SetWindowPosition(pVideo, 0, 0, display_w, display_h);
            }
        }

        IVideoWindow_put_Visible(pVideo, OATRUE);
        Log_Printf(L"DirectShow: Video window set to visible (DXVA2 mode)");
    }

    /* Show registered video decoders in log */
    ds_enum_filters(1);

    g_playing = 0;
    if (!pRenderer) g_renderer_type = 0;  /* Fallback to default renderer */
    return 0;
}

int ds_play(void)
{
    if (!pControl) return -1;
    HRESULT hr = IMediaControl_Run(pControl);
    if (SUCCEEDED(hr)) { g_playing = 1; return 0; }
    Log_Printf(L"DirectShow: Run failed: 0x%08l", hr);
    return -1;
}

void ds_stop(void)
{
    if (pControl) {
        IMediaControl_Stop(pControl);
    }
    ds_cleanup();
}

int ds_is_playing(void)
{
    if (!pEvent || !g_playing) return 0;

    /* Check for end-of-file event */
    long evCode = 0;
    LONG_PTR p1 = 0, p2 = 0;
    while (IMediaEvent_GetEvent(pEvent, &evCode, &p1, &p2, 0) == S_OK) {
        IMediaEvent_FreeEventParams(pEvent, evCode, p1, p2);
        if (evCode == EC_COMPLETE || evCode == EC_ERRORABORT) {
            g_playing = 0;
            return 0;
        }
    }
    return g_playing;
}

double ds_get_position(void)
{
    if (!pSeeking) return 0.0;
    LONGLONG pos = 0;
    IMediaSeeking_GetCurrentPosition(pSeeking, &pos);
    return (double)pos / 10000000.0;  /* Convert 100-ns to seconds */
}

double ds_get_duration(void)
{
    if (!pSeeking) return 0.0;
    LONGLONG dur = 0;
    IMediaSeeking_GetDuration(pSeeking, &dur);
    return (double)dur / 10000000.0;
}

void ds_set_volume(float vol)
{
    if (!pAudio) return;
    /* IBasicAudio volume range: -10000 (silence) to 0 (full) */
    long v = (long)((vol - 1.0f) * 10000.0f);
    if (v < -10000) v = -10000;
    if (v > 0) v = 0;
    IBasicAudio_put_Volume(pAudio, v);
}

void ds_resize(int x, int y, int w, int h)
{
    if (!pVideo || w <= 0 || h <= 0) return;

    /* Fill background black */
    if (g_hwndDisplay) {
        HDC hdc = GetDC(g_hwndDisplay);
        if (hdc) {
            RECT rc;
            GetClientRect(g_hwndDisplay, &rc);
            HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));
            RECT full = {0, 0, rc.right, rc.bottom};
            FillRect(hdc, &full, brush);
            DeleteObject(brush);
            ReleaseDC(g_hwndDisplay, hdc);
        }
    }

    if (g_video_w > 0 && g_video_h > 0) {
        /* Aspect-ratio preserving letterbox */
        float scale_x = (float)w / (float)g_video_w;
        float scale_y = (float)h / (float)g_video_h;
        float scale = (scale_x < scale_y) ? scale_x : scale_y;
        int draw_w = (int)(g_video_w * scale);
        int draw_h = (int)(g_video_h * scale);
        int draw_x = x + (w - draw_w) / 2;
        int draw_y = y + (h - draw_h) / 2;

        IVideoWindow_SetWindowPosition(pVideo, draw_x, draw_y, draw_w, draw_h);
    } else {
        /* No native size info, fill window */
        IVideoWindow_SetWindowPosition(pVideo, x, y, w, h);
    }
}

int ds_is_using_dxva2(void)
{
    return g_dxva2;
}

int ds_has_video(void)
{
    return (g_video_w > 0 && g_video_h > 0) ? 1 : 0;
}

int ds_get_video_width(void)
{
    return g_video_w;
}

int ds_get_video_height(void)
{
    return g_video_h;
}

/* Helper: convert CLSID to string */
static void ds_clsid_to_string(REFCLSID clsid, wchar_t *buf, int buf_len)
{
    StringFromGUID2(clsid, buf, buf_len);
}

/* Helper: enumerate filters in a category from registry */
static int ds_enum_category_filters(REFCLSID pCategoryClsid, const wchar_t *category_name)
{
    HKEY hKey;
    wchar_t keyPath[256];
    wchar_t clsidStr[64];
    int count = 0;

    /* Build registry path: CLSID\{category-clsid}\Instance */
    ds_clsid_to_string(pCategoryClsid, clsidStr, 64);
    swprintf(keyPath, 256, L"CLSID\\%ls\\Instance", clsidStr);

    if (RegOpenKeyExW(HKEY_CLASSES_ROOT, keyPath, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
        Log_Printf(L"[DS Filters] Cannot open category: %ls", category_name);
        return 0;
    }

    Log_Printf(L"=== %ls ===", category_name);

    DWORD index = 0;
    wchar_t subKeyName[256];
    DWORD subKeyLen = 256;

    while (RegEnumKeyExW(hKey, index, subKeyName, &subKeyLen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
        HKEY hSubKey;
        wchar_t subKeyPath[512];
        swprintf(subKeyPath, 512, L"%ls\\%ls", keyPath, subKeyName);

        if (RegOpenKeyExW(HKEY_CLASSES_ROOT, subKeyPath, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
            /* Read filter name */
            wchar_t filterName[256] = L"(unknown)";
            DWORD nameLen = sizeof(filterName);
            RegQueryValueExW(hSubKey, L"FriendlyName", NULL, NULL, (LPBYTE)filterName, &nameLen);

            /* Read CLSID string */
            wchar_t filterClsid[64] = L"";
            DWORD clsidLen = sizeof(filterClsid);
            RegQueryValueExW(hSubKey, L"CLSID", NULL, NULL, (LPBYTE)filterClsid, &clsidLen);

            Log_Printf(L"  [%ls] %ls", filterClsid, filterName);
            count++;

            RegCloseKey(hSubKey);
        }

        subKeyLen = 256;
        index++;
    }

    RegCloseKey(hKey);
    return count;
}

int ds_enum_filters(int category)
{
    int total = 0;

    Log_Printf(L"");
    Log_Printf(L"============ DirectShow Filter Registration Info ============");
    Log_Printf(L"");

    if (category == 0 || category == 1) {
        total += ds_enum_category_filters(&CLSID_VideoCompressorCategory, L"Video Compressors / Decompressors");
    }
    if (category == 0 || category == 2) {
        total += ds_enum_category_filters(&CLSID_AudioCompressorCategory, L"Audio Compressors / Decompressors");
    }
    if (category == 0 || category == 3) {
        total += ds_enum_category_filters(&CLSID_LegacyAmFilterCategory, L"DirectShow Filters (Legacy)");
    }

    Log_Printf(L"");
    Log_Printf(L"Total filters found: %d", total);
    Log_Printf(L"============================================================");
    Log_Printf(L"");

    return total;
}

const wchar_t *ds_get_renderer_name(void)
{
    if (!g_playing) return L"None";
    switch (g_renderer_type) {
        case 1:  return L"VMR-9";
        case 2:  return L"EVR";
        default: return L"Default";
    }
}
