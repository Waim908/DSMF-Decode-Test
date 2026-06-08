/*
 * DXVA2 Hardware Acceleration Helper
 * Manages Direct3D9 device for DXVA2 video processing.
 */
#include "dxva2_helper.h"

#include <d3d9.h>
#include <dxva2api.h>
#include <stdio.h>

/* IID for IDirectXVideoProcessorService - may be missing from MinGW dxva2 libs */
static const GUID IID_IDXVPS = {
    0xfc51a552, 0xd5e8, 0x11d2, {0xaf, 0x46, 0x00, 0xc0, 0x4f, 0x72, 0xb7, 0x69}
};

static IDirect3D9            *pD3D        = NULL;
static IDirect3DDevice9      *pDevice     = NULL;
static IDirect3DSurface9     *pRT         = NULL;
static D3DPRESENT_PARAMETERS  d3dpp;
static HWND                   g_hwnd      = NULL;
static int                    g_width     = 0;
static int                    g_height    = 0;

IDirect3DDevice9 *dxva2_init(HWND hwnd, int width, int height)
{
    D3DDISPLAYMODE dmode;

    dxva2_cleanup();

    if (!hwnd || width <= 0 || height <= 0)
        return NULL;

    pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (!pD3D) {
        fprintf(stderr, "DXVA2: Direct3DCreate9 failed\n");
        return NULL;
    }

    /* Get current display mode */
    if (IDirect3D9_GetAdapterDisplayMode(pD3D, D3DADAPTER_DEFAULT, &dmode) != D3D_OK) {
        fprintf(stderr, "DXVA2: GetAdapterDisplayMode failed\n");
        dxva2_cleanup();
        return NULL;
    }

    /* Setup present parameters */
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed               = TRUE;
    d3dpp.hDeviceWindow          = hwnd;
    d3dpp.BackBufferWidth        = width;
    d3dpp.BackBufferHeight       = height;
    d3dpp.BackBufferFormat       = dmode.Format;
    d3dpp.BackBufferCount        = 1;
    d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_ONE;
    d3dpp.Flags                  = D3DPRESENTFLAG_VIDEO;

    /* Create device with hardware vertex processing */
    HRESULT hr = IDirect3D9_CreateDevice(pD3D, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
        D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE,
        &d3dpp, &pDevice);

    if (FAILED(hr)) {
        /* Fallback to software vertex processing */
        hr = IDirect3D9_CreateDevice(pD3D, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd,
            D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED | D3DCREATE_FPU_PRESERVE,
            &d3dpp, &pDevice);
    }

    if (FAILED(hr)) {
        fprintf(stderr, "DXVA2: CreateDevice failed: 0x%08lx\n", hr);
        dxva2_cleanup();
        return NULL;
    }

    /* Create render target surface */
    hr = IDirect3DDevice9_CreateRenderTarget(pDevice, width, height,
        dmode.Format, D3DMULTISAMPLE_NONE, 0, FALSE, &pRT, NULL);
    if (FAILED(hr)) {
        fprintf(stderr, "DXVA2: CreateRenderTarget failed: 0x%08lx\n", hr);
        /* Non-fatal: we can still use the back buffer */
        pRT = NULL;
    }

    g_hwnd   = hwnd;
    g_width  = width;
    g_height = height;

    fprintf(stdout, "DXVA2: D3D9 device created successfully (%dx%d)\n", width, height);
    return pDevice;
}

int dxva2_check_support(void)
{
    IDirectXVideoProcessorService *processor = NULL;
    GUID *guids = NULL;
    UINT count = 0;
    HRESULT hr;
    int supported = 0;

    /* Need a device first - try a temporary one */
    if (!pDevice) {
        /* Just check if D3D9 can be created */
        IDirect3D9 *d3d = Direct3DCreate9(D3D_SDK_VERSION);
        if (!d3d) return 0;
        IDirect3D9_Release(d3d);
        return 1;  /* D3D9 available, DXVA2 likely supported */
    }

    /* Create DXVA2 video processor service */
    hr = DXVA2CreateVideoService(pDevice, &IID_IDXVPS,
                                 (void **)&processor);
    if (FAILED(hr)) {
        fprintf(stderr, "DXVA2: DXVA2CreateVideoService failed: 0x%08lx\n", hr);
        return 0;
    }

    /* Enumerate video processor GUIDs */
    hr = IDirectXVideoProcessorService_GetVideoProcessorDeviceGuids(processor, NULL, &count, &guids);
    if (SUCCEEDED(hr) && count > 0) {
        supported = 1;
        CoTaskMemFree(guids);
    }

    IDirectXVideoProcessorService_Release(processor);
    return supported;
}

const wchar_t *dxva2_get_device_info(void)
{
    static wchar_t info[256] = {0};
    D3DADAPTER_IDENTIFIER9 id;

    if (!pD3D) {
        swprintf(info, 256, L"DXVA2: Not initialized");
        return info;
    }

    if (IDirect3D9_GetAdapterIdentifier(pD3D, D3DADAPTER_DEFAULT, 0, &id) == D3D_OK) {
        /* Convert description to wide string */
        wchar_t desc[128] = {0};
        MultiByteToWideChar(CP_ACP, 0, id.Description, -1, desc, 128);
        swprintf(info, 256, L"DXVA2: %s (Vendor: 0x%04X, Device: 0x%04X)",
                 desc, id.VendorId, id.DeviceId);
    } else {
        swprintf(info, 256, L"DXVA2: Device info unavailable");
    }
    return info;
}

IDirect3DDevice9 *dxva2_get_device(void)
{
    return pDevice;
}

int dxva2_present(void)
{
    if (!pDevice) return -1;

    HRESULT hr = IDirect3DDevice9_TestCooperativeLevel(pDevice);
    if (hr == D3DERR_DEVICELOST) return -1;
    if (hr == D3DERR_DEVICENOTRESET) {
        hr = IDirect3DDevice9_Reset(pDevice, &d3dpp);
        if (FAILED(hr)) return -1;
    }

    hr = IDirect3DDevice9_Present(pDevice, NULL, NULL, g_hwnd, NULL);
    return SUCCEEDED(hr) ? 0 : -1;
}

IDirect3DSurface9 *dxva2_get_render_target(void)
{
    if (pRT) return pRT;
    if (!pDevice) return NULL;

    /* Get back buffer */
    IDirect3DSurface9 *bb = NULL;
    IDirect3DDevice9_GetBackBuffer(pDevice, 0, 0, D3DBACKBUFFER_TYPE_MONO, &bb);
    return bb;  /* Caller should release */
}

void dxva2_cleanup(void)
{
    if (pRT)    { IDirect3DSurface9_Release(pRT);    pRT    = NULL; }
    if (pDevice){ IDirect3DDevice9_Release(pDevice);  pDevice = NULL; }
    if (pD3D)   { IDirect3D9_Release(pD3D);           pD3D   = NULL; }
    g_hwnd   = NULL;
    g_width  = 0;
    g_height = 0;
}