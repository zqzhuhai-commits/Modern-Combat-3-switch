/* SPDX-License-Identifier: MIT. video/MyVideoView -- mission briefings,
 * skipped cleanly. playMovie asks the engine's own Java_video_MyVideoView_
 * SetFinished export to close the briefing after a short grace period so a
 * black screen never hangs a mission. Reference: EapRules jni/video_player.cpp.
 */
#include "mc3.h"
#include "jni.h"
static int pending;
static unsigned tick;
void mc3_video_play(void) { pending = 1; tick = 0; }
void mc3_video_tick(void) {
    if (!pending) return;
    if (++tick < 90) return; /* ~1.5s at 60fps, then skip */
    pending = 0;
    if (g_mc3_api.video_finished) {
        typedef void (*sf_fn)(void *, void *);
        ((sf_fn)g_mc3_api.video_finished)(g_jni_env, (void *)(uintptr_t)0x42424242);
    }
}
