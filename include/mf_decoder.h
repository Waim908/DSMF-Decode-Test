#ifndef MF_DECODER_H
#define MF_DECODER_H

#include <windows.h>

/*
 * Media Foundation video decoder.
 * Uses IMFSourceReader to decode video frames.
 *
 * enable_dxva2: if non-zero, attempt to use DXVA2 hardware acceleration.
 * Returns 0 on success, non-zero on failure.
 */
int mf_open(const wchar_t *filepath, HWND hwnd_display, int enable_dxva2);

/* Decode and render next frame. Returns 0 on success, 1 on EOF, -1 on error */
int mf_render_next_frame(void);

/* Stop and release all resources */
void mf_stop(void);

/* Check if decoder is active */
int mf_is_active(void);

/* Get current position in 100-ns units */
long long mf_get_position(void);

/* Get duration in 100-ns units */
long long mf_get_duration(void);

/* Check if DXVA2 is being used */
int mf_is_using_dxva2(void);

/* Get decoder description */
const wchar_t *mf_get_decoder_info(void);

#endif /* MF_DECODER_H */
