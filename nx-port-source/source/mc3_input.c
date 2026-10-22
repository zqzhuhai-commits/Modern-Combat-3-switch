/* SPDX-License-Identifier: MIT. MC3 twin-stick controller mapping; reference:
 * EapRules mc3-native-arm input_bridge.cpp (verified on hardware): Xperia Play
 * path -- GL2JNILib.nativeSetTouchPadDTLeft(x,-y,1), nativeSetTouchPadDT(x,y,2)
 * in -1..1, plus Android keycodes and 640x480 touch events.
 */
#include "mc3.h"
#include "jni.h"
#include <switch.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define K_BACK 4
#define K_UP 19
#define K_DOWN 20
#define K_LEFT 21
#define K_RIGHT 22
#define K_CENTER 23
#define K_BTN_X 99
#define K_BTN_Y 100
#define K_BTN_L1 102
#define K_BTN_R1 103
#define K_BTN_L2 104
#define K_BTN_R2 105
#define K_START 108
#define K_SELECT 109
#define K_MOVE_END 123

typedef void (*key_fn)(void *, void *, int, int);
typedef void (*touch_fn)(void *, void *, int, int, int, int);
typedef void (*pad_fn)(void *, void *, float, float, int);

static PadState pad;
static HidTouchScreenState previous;
static key_fn do_key;
static touch_fn do_touch;
static pad_fn do_pad_left, do_pad_right;
static int joy;
static int cx = MC3_WIDTH / 2, cy = MC3_HEIGHT / 2, click;
static int swiping, swipe_to, swipe_tick;
static int s_a, s_b, s_x, s_y, s_l1, s_r1, s_l2, s_r2, s_start, s_select;
static int s_up, s_down, s_left, s_right;

void mc3_input_init(void) {
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&pad);
    hidInitializeTouchScreen();
    memset(&previous, 0, sizeof previous);
    do_key = (key_fn)g_mc3_api.keyboard;
    do_touch = (touch_fn)g_mc3_api.touch;
    do_pad_left = (pad_fn)g_mc3_api.pad_left;
    do_pad_right = (pad_fn)g_mc3_api.pad_right;
}

static void key(int code, int down) { if (do_key) do_key(g_jni_env, NULL, code, down); }
static void touch(int type, int x, int y, int id) { if (do_touch) do_touch(g_jni_env, NULL, type, x, y, id); }
static void pad_left(float x, float y, int id) { if (do_pad_left) do_pad_left(g_jni_env, NULL, x, -y, id); }
static void pad_right(float x, float y, int id) { if (do_pad_right) do_pad_right(g_jni_env, NULL, x, y, id); }

static void held_key(u64 keys, u64 mask, int code, int *old) {
    int active = (keys & mask) != 0;
    if (active != *old) key(code, active);
    *old = active;
}

static void swipe_end(void) {
    if (swiping && swipe_tick >= 0) { touch(0, MC3_WIDTH / 2, swipe_to, 60); swiping = 0; }
}

void mc3_input_release(void) {
    if (joy) pad_left(0, 0, 1);
    joy = 0;
    swipe_end();
    if (click) touch(0, cx, cy, 55);
    click = 0;
    for (int i = 0; i < previous.count; i++)
        touch(0, previous.touches[i].x * MC3_WIDTH / 1280,
              previous.touches[i].y * MC3_HEIGHT / 720,
              (int)previous.touches[i].finger_id);
    previous.count = 0;
}

int mc3_input_poll(void) {
    padUpdate(&pad);
    u64 keys = padGetButtons(&pad), down = padGetButtonsDown(&pad), up = padGetButtonsUp(&pad);
    if ((keys & HidNpadButton_Plus) && (keys & HidNpadButton_Minus)) return 1;
    /* A = confirm, B = back, X = reload, Y = special, L1 = aim, R1 = fire,
     * ZL = grenade, ZR = reload, Plus = start, Minus = select. */
    held_key(keys, HidNpadButton_A, K_CENTER, &s_a);
    held_key(keys, HidNpadButton_B, K_MOVE_END, &s_b);
    held_key(keys, HidNpadButton_X, K_BTN_X, &s_x);
    held_key(keys, HidNpadButton_Y, K_BTN_Y, &s_y);
    held_key(keys, HidNpadButton_L, K_BTN_L1, &s_l1);
    held_key(keys, HidNpadButton_R, K_BTN_R1, &s_r1);
    held_key(keys, HidNpadButton_ZL, K_MOVE_END, &s_l2);
    held_key(keys, HidNpadButton_ZR, K_BTN_X, &s_r2);
    held_key(keys, HidNpadButton_Plus, K_START, &s_start);
    held_key(keys, HidNpadButton_Minus, K_SELECT, &s_select);
    held_key(keys, HidNpadButton_Up, K_UP, &s_up);
    held_key(keys, HidNpadButton_Down, K_DOWN, &s_down);
    held_key(keys, HidNpadButton_Left, K_LEFT, &s_left);
    held_key(keys, HidNpadButton_Right, K_RIGHT, &s_right);
    /* dpad-down is a cover/vault swipe in gameplay (the EapRules gesture). */
    if (down & HidNpadButton_Down) {
        touch(1, MC3_WIDTH / 2, MC3_HEIGHT / 2, 60);
        swiping = 1; swipe_to = MC3_HEIGHT - 80; swipe_tick = 0;
    }
    if (swiping && ++swipe_tick >= 6) swipe_end();
    HidAnalogStickState l = padGetStickPos(&pad, 0), r = padGetStickPos(&pad, 1);
    float lx = l.x / 32768.0f, ly = l.y / 32768.0f;
    if (abs(l.x) > 3000 || abs(l.y) > 3000) { pad_left(lx, ly, 1); joy = 1; }
    else if (joy) { pad_left(0, 0, 1); joy = 0; }
    float rx = r.x / 32768.0f, ry = r.y / 32768.0f;
    if (abs(r.x) > 3000 || abs(r.y) > 3000) pad_right(rx, ry, 2);
    else pad_right(0, 0, 2);
    /* D-pad also nudges the touch cursor (menus). */
    if (keys & HidNpadButton_Left) cx -= 6;
    if (keys & HidNpadButton_Right) cx += 6;
    if (keys & HidNpadButton_Up) cy -= 6;
    if (keys & HidNpadButton_Down) cy += 6;
    if (cx < 0) cx = 0;
    if (cx >= MC3_WIDTH) cx = MC3_WIDTH - 1;
    if (cy < 0) cy = 0;
    if (cy >= MC3_HEIGHT) cy = MC3_HEIGHT - 1;
    if (down & HidNpadButton_A) { touch(1, cx, cy, 55); click = 1; }
    if (click && (keys & HidNpadButton_A)) touch(2, cx, cy, 55);
    if (up & HidNpadButton_A) { touch(0, cx, cy, 55); click = 0; }
    HidTouchScreenState now = {0};
    hidGetTouchScreenStates(&now, 1);
    for (int i = 0; i < previous.count; i++) {
        int found = 0;
        for (int j = 0; j < now.count; j++) found |= previous.touches[i].finger_id == now.touches[j].finger_id;
        if (!found) touch(0, previous.touches[i].x * MC3_WIDTH / 1280,
                          previous.touches[i].y * MC3_HEIGHT / 720,
                          (int)previous.touches[i].finger_id);
    }
    for (int i = 0; i < now.count; i++) {
        int found = 0;
        for (int j = 0; j < previous.count; j++) found |= now.touches[i].finger_id == previous.touches[j].finger_id;
        touch(found ? 2 : 1, now.touches[i].x * MC3_WIDTH / 1280,
              now.touches[i].y * MC3_HEIGHT / 720,
              (int)now.touches[i].finger_id);
    }
    previous = now;
    return 0;
}
