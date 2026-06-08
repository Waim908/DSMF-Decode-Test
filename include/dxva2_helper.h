#ifndef DXVA2_HELPER_H
#define DXVA2_HELPER_H

#include <windows.h>
#include <d3d9.h>

/*
 * DXVA2 hardware acceleration helper.
 * Manages Direct3D9 device and DXVA2 video processor.
 */

/* Initialize DXVA2 with a HWND for rendering.
 * Returns D3D device pointer on success, NULL on failure. */
IDirect3DDevice9 *dxva2_init(HWND hwnd, int width, int height);

/* Check if DXVA2 hardware acceleration is available */
int dxva2_check_support(void);

/* Get DXVA2 device description */
const wchar_t *dxva2_get_device_info(void);

/* Cleanup DXVA2 resources */
void dxva2_cleanup(void);

/* Get the current D3D device (NULL if not initialized) */
IDirect3DDevice9 *dxva2_get_device(void);

/* Present the current back buffer */
int dxva2_present(void);

/* Get a surface for rendering */
IDirect3DSurface9 *dxva2_get_render_target(void);

#endif /* DXVA2_HELPER_H */
