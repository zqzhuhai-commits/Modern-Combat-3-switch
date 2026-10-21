/* SPDX-License-Identifier: MIT */
#ifndef MC3_API_H
#define MC3_API_H
#include <stdint.h>
typedef uintptr_t (*mc3_lookup)(void *module, const char *symbol);
typedef void (*mc3_log)(const char *symbol);
typedef struct {
    /* module entry points */
    uintptr_t on_load;                  /* JNI_OnLoad */
    uintptr_t gl_init;                  /* GL2JNILib.init */
    uintptr_t billing_init;             /* iab.InAppBilling.nativeInit */
    uintptr_t video_init;               /* video.MyVideoView.Init */
    uintptr_t game_init;                /* GloftM3HM.Init (activity instance) */
    uintptr_t game_mc3_init;            /* GloftM3HM.nativeMC3Init */
    uintptr_t view_settings;            /* GL2JNILib.getViewSettings */
    uintptr_t resize;                   /* GL2JNILib.resize */
    uintptr_t step;                     /* GL2JNILib.step */
    uintptr_t touch;                    /* GL2JNILib.touchEvent */
    uintptr_t keyboard;                 /* GL2JNILib.keyboardEvent */
    uintptr_t pad_left;                 /* GL2JNILib.nativeSetTouchPadDTLeft */
    uintptr_t pad_right;                /* GL2JNILib.nativeSetTouchPadDT */
    uintptr_t is_gameplay;              /* GloftM3HM.isGamePlay */
    uintptr_t video_finished;           /* video.MyVideoView.SetFinished */
} Mc3Api;
int mc3_bind(Mc3Api *api, mc3_lookup lookup, void *module, mc3_log log);
#endif
