/* SPDX-License-Identifier: MIT */
#ifndef MC3_H
#define MC3_H
#include "config.h"
#include "mc3_api.h"
#include <stddef.h>
#define MC3_WIDTH  640
#define MC3_HEIGHT 480
#define MC3_OUTPUT_WIDTH  RT_SCREEN_W
#define MC3_OUTPUT_HEIGHT RT_SCREEN_H
#define MC3_PKG "com.gameloft.android.ANMP.GloftM3HM"
#define MC3_CLASS "com/gameloft/android/ANMP/GloftM3HM/"
#define MC3_MAIN_OBB "main.1120.com.gameloft.android.ANMP.GloftM3HM.obb"
#define MC3_PATCH_OBB "patch.11428.com.gameloft.android.ANMP.GloftM3HM.obb"
extern Mc3Api g_mc3_api;
extern void *g_jni_env;
#include "so_util.h"
extern void mc3_unlock_drm(so_module *mod);
extern void mc3_swp_region(uint32_t begin, uint32_t end);
extern uint32_t mc3_swp_count(void);
extern void mc3_input_init(void);
extern int  mc3_input_poll(void);
extern void mc3_input_release(void);
extern void mc3_audio_pause(int pause);
extern void mc3_audio_close(void);
extern void mc3_video_tick(void);
extern void mc3_jni_check(const char *stage);
#endif
