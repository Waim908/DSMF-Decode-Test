/*
 * DirectShow Video Decoder
 * Uses IGraphBuilder + IMediaControl + IVideoWindow for playback.
 */
#include "directshow_decoder.h"

#include <dshow.h>
#include <stdio.h>

/* CLSID_EnhancedVideoRenderer may not be in dshow.h */
#ifndef CLSID_EnhancedVideoRenderer
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
}

int ds_open(const wchar_t *filepath, HWND hwnd_display)
{
    HRESULT hr;
    IBaseFilter *pSource = NULL;

    ds_cleanup();

    /* Create the Filter Graph Manager */
    hr = CoCreateInstance(&CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER,
                          &IID_IGraphBuilder, (void **)&pGraph);
    if (FAILED(hr)) { fprintf(stderr, "DirectShow: Failed to create FilterGraph: 0x%08lx\n", hr); return -1; }

    /* Query interfaces */
    hr = IGraphBuilder_QueryInterface(pGraph, &IID_IMediaControl, (void **)&pControl);
    if (FAILED(hr)) fprintf(stderr, "DirectShow: Failed to query IMediaControl: 0x%08lx\n", hr);
    hr = IGraphBuilder_QueryInterface(pGraph, &IID_IMediaEvent,   (void **)&pEvent);
    if (FAILED(hr)) fprintf(stderr, "DirectShow: Failed to query IMediaEvent: 0x%08lx\n", hr);
    hr = IGraphBuilder_QueryInterface(pGraph, &IID_IVideoWindow,  (void **)&pVideo);
    if (FAILED(hr)) fprintf(stderr, "DirectShow: Failed to query IVideoWindow: 0x%08lx\n", hr);
    hr = IGraphBuilder_QueryInterface(pGraph, &IID_IBasicVideo,   (void **)&pBasicVideo);
    if (FAILED(hr)) fprintf(stderr, "DirectShow: Failed to query IBasicVideo: 0x%08lx\n", hr);
    hr = IGraphBuilder_QueryInterface(pGraph, &IID_IBasicAudio,   (void **)&pAudio);
    if (FAILED(hr)) fprintf(stderr, "DirectShow: Failed to query IBasicAudio: 0x%08lx\n", hr);
    hr = IGraphBuilder_QueryInterface(pGraph, &IID_IMediaSeeking, (void **)&pSeeking);
    if (FAILED(hr)) fprintf(stderr, "DirectShow: Failed to query IMediaSeeking: 0x%08lx\n", hr);

    if (!pControl || !pVideo) {
        fprintf(stderr, "DirectShow: Failed to query required interfaces\n");
        ds_cleanup();
        return -1;
    }

    /* Add source filter for the file */
    hr = IGraphBuilder_AddSourceFilter(pGraph, filepath, L"Source", &pSource);
    if (FAILED(hr)) {
        fprintf(stderr, "DirectShow: Failed to add source filter for '%ls': 0x%08lx\n", filepath, hr);
        ds_cleanup();
        return -1;
    }

    /* Render all output pins (auto-connects decoder + renderer) */
    hr = IGraphBuilder_RenderFile(pGraph, filepath, NULL);
    if (FAILED(hr)) {
        fprintf(stderr, "DirectShow: RenderFile failed: 0x%08lx\n", hr);
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
                fprintf(stdout, "DirectShow: Native video size %ldx%ld\n", vw, vh);
            }
        }

        /* Fit video to display area with aspect ratio */
        ds_update_aspect();
        IVideoWindow_put_Visible(pVideo, OATRUE);
    }

    g_playing = 0;
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
    if (FAILED(hr)) { fprintf(stderr, "DirectShow: Failed to create FilterGraph: 0x%08lx\n", hr); return -1; }

    /* Query interfaces */
    hr = IGraphBuilder_QueryInterface(pGraph, &IID_IMediaControl, (void **)&pControl);
    if (FAILED(hr)) fprintf(stderr, "DirectShow: Failed to query IMediaControl: 0x%08lx\n", hr);
    hr = IGraphBuilder_QueryInterface(pGraph, &IID_IMediaEvent,   (void **)&pEvent);
    if (FAILED(hr)) fprintf(stderr, "DirectShow: Failed to query IMediaEvent: 0x%08lx\n", hr);
    hr = IGraphBuilder_QueryInterface(pGraph, &IID_IVideoWindow,  (void **)&pVideo);
    if (FAILED(hr)) fprintf(stderr, "DirectShow: Failed to query IVideoWindow: 0x%08lx\n", hr);
    hr = IGraphBuilder_QueryInterface(pGraph, &IID_IBasicVideo,   (void **)&pBasicVideo);
    if (FAILED(hr)) fprintf(stderr, "DirectShow: Failed to query IBasicVideo: 0x%08lx\n", hr);
    hr = IGraphBuilder_QueryInterface(pGraph, &IID_IBasicAudio,   (void **)&pAudio);
    if (FAILED(hr)) fprintf(stderr, "DirectShow: Failed to query IBasicAudio: 0x%08lx\n", hr);
    hr = IGraphBuilder_QueryInterface(pGraph, &IID_IMediaSeeking, (void **)&pSeeking);
    if (FAILED(hr)) fprintf(stderr, "DirectShow: Failed to query IMediaSeeking: 0x%08lx\n", hr);

    if (!pControl || !pVideo) {
        fprintf(stderr, "DirectShow: Failed to query required interfaces\n");
        ds_cleanup();
        return -1;
    }

    /* Add source filter for the file */
    hr = IGraphBuilder_AddSourceFilter(pGraph, filepath, L"Source", &pSource);
    if (FAILED(hr)) {
        fprintf(stderr, "DirectShow: Failed to add source filter for '%ls': 0x%08lx\n", filepath, hr);
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
                fprintf(stdout, "DirectShow: Using VMR-9 renderer (DXVA2 capable, MinGW build)\n");
#else
                fprintf(stdout, "DirectShow: Using VMR-9 renderer (DXVA2 capable)\n");
#endif
                g_dxva2 = 1;
            } else {
                fprintf(stderr, "DirectShow: Failed to add VMR-9 filter: 0x%08lx\n", hr);
                IBaseFilter_Release(pRenderer);
                pRenderer = NULL;
            }
        } else {
            fprintf(stderr, "DirectShow: Failed to create VMR-9: 0x%08lx\n", hr);
        }

        /* If VMR-9 failed, try EVR */
        if (!pRenderer) {
            hr = CoCreateInstance(&CLSID_EnhancedVideoRenderer, NULL, CLSCTX_INPROC_SERVER,
                                  &IID_IBaseFilter, (void **)&pRenderer);
            if (SUCCEEDED(hr)) {
                hr = IGraphBuilder_AddFilter(pGraph, pRenderer, L"EVR");
                if (SUCCEEDED(hr)) {
#ifdef __MINGW32__
                    fprintf(stdout, "DirectShow: Using EVR renderer (DXVA2 capable, MinGW build)\n");
#else
                    fprintf(stdout, "DirectShow: Using EVR renderer (DXVA2 capable)\n");
#endif
                    g_dxva2 = 1;
                } else {
                    fprintf(stderr, "DirectShow: Failed to add EVR filter: 0x%08lx\n", hr);
                    IBaseFilter_Release(pRenderer);
                    pRenderer = NULL;
                }
            } else {
                fprintf(stderr, "DirectShow: Failed to create EVR: 0x%08lx\n", hr);
            }
        }
    }

    /* Render all output pins (auto-connects decoder + renderer) */
    hr = IGraphBuilder_RenderFile(pGraph, filepath, NULL);
    if (FAILED(hr)) {
        fprintf(stderr, "DirectShow: RenderFile failed: 0x%08lx\n", hr);
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
                fprintf(stdout, "DirectShow: Native video size %ldx%ld\n", vw, vh);
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
                fprintf(stdout, "DirectShow: Video position set to %dx%d at (%d,%d)\n", draw_w, draw_h, draw_x, draw_y);
            } else {
                /* No video size info, fill the window */
                IVideoWindow_SetWindowPosition(pVideo, 0, 0, display_w, display_h);
            }
        }

        IVideoWindow_put_Visible(pVideo, OATRUE);
    }

    g_playing = 0;
    return 0;
}

int ds_play(void)
{
    if (!pControl) return -1;
    HRESULT hr = IMediaControl_Run(pControl);
    if (SUCCEEDED(hr)) { g_playing = 1; return 0; }
    fprintf(stderr, "DirectShow: Run failed: 0x%08lx\n", hr);
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
