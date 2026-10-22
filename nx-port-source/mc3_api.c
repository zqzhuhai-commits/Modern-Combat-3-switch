/* SPDX-License-Identifier: MIT */
#include "mc3_api.h"
#include <string.h>

#define MC3_PREFIX_CLASS "com/gameloft/glf/"
#define MC3_PREFIX "Java_com_gameloft_"

int mc3_bind(Mc3Api *api, mc3_lookup lookup, void *module, mc3_log log) {
    int missing = 0;
    if (!api || !lookup) return -1;
    memset(api, 0, sizeof(*api));
#define BIND(field, symbol) do { \
    uintptr_t addr = lookup(module, symbol); \
    _Static_assert(sizeof(api->field) == sizeof(addr), "unsupported pointer ABI"); \
    memcpy(&api->field, &addr, sizeof(addr)); \
    if (!addr) { ++missing; if (log) log(symbol); } \
} while (0)
    BIND(on_load, "JNI_OnLoad");
    BIND(gl_init, MC3_PREFIX "glf_GL2JNILib_init");
    BIND(billing_init, MC3_PREFIX "android_ANMP_GloftM3HM_iab_InAppBilling_nativeInit");
    BIND(video_init, MC3_PREFIX "video_MyVideoView_Init");
    BIND(game_init, MC3_PREFIX "android_ANMP_GloftM3HM_GloftM3HM_Init");
    BIND(game_mc3_init, MC3_PREFIX "android_ANMP_GloftM3HM_GloftM3HM_nativeMC3Init");
    BIND(view_settings, MC3_PREFIX "glf_GL2JNILib_getViewSettings");
    BIND(resize, MC3_PREFIX "glf_GL2JNILib_resize");
    BIND(step, MC3_PREFIX "glf_GL2JNILib_step");
    BIND(touch, MC3_PREFIX "glf_GL2JNILib_touchEvent");
    BIND(keyboard, MC3_PREFIX "glf_GL2JNILib_keyboardEvent");
    BIND(pad_left, MC3_PREFIX "glf_GL2JNILib_nativeSetTouchPadDTLeft");
    BIND(pad_right, MC3_PREFIX "glf_GL2JNILib_nativeSetTouchPadDT");
    BIND(is_gameplay, MC3_PREFIX "android_ANMP_GloftM3HM_GloftM3HM_isGamePlay");
    BIND(video_finished, MC3_PREFIX "video_MyVideoView_SetFinished");
#undef BIND
    return missing;
}
