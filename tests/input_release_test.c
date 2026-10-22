#include "mc3.h"
#include <switch.h>
#include <assert.h>
#include <stdio.h>
Mc3Api g_mc3_api;
void *g_jni_env;
static u64 keys,downs,ups;
static int key_state[256],touches[64];
static float left_x,right_x;
void padConfigureInput(int a,int b){(void)a;(void)b;}
void padInitializeDefault(PadState *p){(void)p;}
void hidInitializeTouchScreen(void){}
void padUpdate(PadState *p){(void)p;}
u64 padGetButtons(PadState *p){(void)p;return keys;}
u64 padGetButtonsDown(PadState *p){(void)p;return downs;}
u64 padGetButtonsUp(PadState *p){(void)p;return ups;}
HidAnalogStickState padGetStickPos(PadState *p,int i){(void)p;(void)i;return (HidAnalogStickState){16000,0};}
void hidGetTouchScreenStates(HidTouchScreenState *s,int n){(void)n;s->count=0;}
static void on_key(void *e,void *c,int code,int down){(void)e;(void)c;assert(code<256);key_state[code]=down;}
static void on_touch(void *e,void *c,int type,int x,int y,int id){(void)e;(void)c;(void)x;(void)y;assert(id<64);touches[id]=type!=0;}
static void on_left(void *e,void *c,float x,float y,int id){(void)e;(void)c;(void)y;(void)id;left_x=x;}
static void on_right(void *e,void *c,float x,float y,int id){(void)e;(void)c;(void)y;(void)id;right_x=x;}
int main(void){
 g_mc3_api.keyboard=(uintptr_t)on_key;g_mc3_api.touch=(uintptr_t)on_touch;g_mc3_api.pad_left=(uintptr_t)on_left;g_mc3_api.pad_right=(uintptr_t)on_right;mc3_input_init();
 keys=downs=HidNpadButton_A|HidNpadButton_R|HidNpadButton_Down;assert(!mc3_input_poll());assert(key_state[23]&&key_state[103]&&key_state[20]);assert(touches[55]&&touches[60]);assert(left_x&&right_x);
 mc3_input_release();for(int i=0;i<256;i++)assert(!key_state[i]);for(int i=0;i<64;i++)assert(!touches[i]);assert(!left_x&&!right_x);
 downs=0;assert(!mc3_input_poll());assert(key_state[23]&&key_state[103]);mc3_input_release();
 keys=downs=HidNpadButton_X|HidNpadButton_ZR;ups=0;mc3_input_poll();assert(key_state[99]);
 keys=HidNpadButton_ZR;downs=0;ups=HidNpadButton_X;mc3_input_poll();assert(key_state[99]);
 keys=0;ups=HidNpadButton_ZR;mc3_input_poll();assert(!key_state[99]);mc3_input_release();
 puts("Input focus-loss release/reacquire tests passed");
}
