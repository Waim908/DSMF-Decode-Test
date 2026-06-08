#ifndef DIRECTSHOW_DECODER_H
#define DIRECTSHOW_DECODER_H

#include <windows.h>

/*
 * DirectShow-based video decoder.
 * Uses IGraphBuilder + IMediaControl + IVideoWindow.
 * Returns 0 on success, non-zero on failure.
 */
int ds_open(const wchar_t *filepath, HWND hwnd_display);

/* Start playback */
int ds_play(void);

/* Stop playback and release all resources */
void ds_stop(void);

/* Check if currently playing */
int ds_is_playing(void);

/* Get current position in seconds */
double ds_get_position(void);

/* Get duration in seconds */
double ds_get_duration(void);

/* Set volume (0.0 - 1.0) */
void ds_set_volume(float vol);

/* Resize the video window to fit the given client area */
void ds_resize(int x, int y, int w, int h);

#endif /* DIRECTSHOW_DECODER_H */
