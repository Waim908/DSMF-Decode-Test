#ifndef LOG_H
#define LOG_H

#include <windows.h>
#include <stdarg.h>

/* Log window handle (set by main.c) */
extern HWND g_hwndLog;

/* Log output function - appends text to GUI log display */
void Log_Append(const wchar_t *text);

/* Log printf-style formatted text */
void Log_Printf(const wchar_t *fmt, ...);

#endif /* LOG_H */
