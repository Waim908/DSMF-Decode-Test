#ifndef LANG_H
#define LANG_H

#include "app_config.h"

/* Language string structure */
typedef struct {
    /* Window title */
    const wchar_t *windowTitle;
    
    /* Buttons */
    const wchar_t *btnDirectShow;
    const wchar_t *btnDsDxva2;
    const wchar_t *btnMfSoftware;
    const wchar_t *btnMfDxva2;
    const wchar_t *btnMfD3d11;
    const wchar_t *btnMfD3d12;
    const wchar_t *btnStop;
    const wchar_t *btnOpenFile;
    const wchar_t *btnAbout;
    const wchar_t *btnOk;
    const wchar_t *btnCancel;
    
    /* Status messages */
    const wchar_t *statusReady;
    const wchar_t *statusStopped;
    const wchar_t *statusFileSelected;
    const wchar_t *statusPlaying;
    const wchar_t *statusPlaybackComplete;
    const wchar_t *statusDecodeError;
    const wchar_t *statusOpening;
    const wchar_t *statusInit;
    const wchar_t *statusHwUnavailable;
    const wchar_t *statusHwInitFailed;
    const wchar_t *statusPureAudio;
    
    /* Hardware tags */
    const wchar_t *hwSoftware;
    const wchar_t *hwDxva2;
    const wchar_t *hwD3d11;
    const wchar_t *hwD3d12;
    const wchar_t *hwAccel;
    const wchar_t *hwFallback;
    
    /* Dialog messages */
    const wchar_t *msgSelectFile;
    const wchar_t *msgFileNotFound;
    const wchar_t *msgCannotPlay;
    const wchar_t *msgPlayFailed;
    const wchar_t *msgError;
    const wchar_t *msgHint;
    
    /* About dialog */
    const wchar_t *aboutTitle;
    const wchar_t *aboutVersion;
    
    /* File dialog */
    const wchar_t *fileDialogTitle;
    const wchar_t *fileTypeVideo;
    const wchar_t *fileTypeAll;

    /* Language settings dialog */
    const wchar_t *langSettingsTitle;
    const wchar_t *langSettingsLabel;
    const wchar_t *langChinese;
    const wchar_t *langEnglish;
    const wchar_t *langSettingsHint;
    const wchar_t *btnLangSettings;
} LangStrings;

/* Get language strings for current language */
const LangStrings* Lang_GetStrings(void);

/* Get current language ID */
int Lang_GetCurrent(void);

/* Set current language */
void Lang_SetCurrent(int lang);

#endif /* LANG_H */
