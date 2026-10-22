#pragma once
#include <stdint.h>
typedef uint64_t u64;
typedef struct {int unused;} PadState;
typedef struct {int x,y;} HidAnalogStickState;
typedef struct {unsigned finger_id;int x,y;} TestTouch;
typedef struct {int count;TestTouch touches[16];} HidTouchScreenState;
enum {HidNpadStyleSet_NpadStandard=1};
#define HidNpadButton_A (1ULL<<0)
#define HidNpadButton_B (1ULL<<1)
#define HidNpadButton_X (1ULL<<2)
#define HidNpadButton_Y (1ULL<<3)
#define HidNpadButton_L (1ULL<<4)
#define HidNpadButton_R (1ULL<<5)
#define HidNpadButton_ZL (1ULL<<6)
#define HidNpadButton_ZR (1ULL<<7)
#define HidNpadButton_Plus (1ULL<<8)
#define HidNpadButton_Minus (1ULL<<9)
#define HidNpadButton_Up (1ULL<<10)
#define HidNpadButton_Down (1ULL<<11)
#define HidNpadButton_Left (1ULL<<12)
#define HidNpadButton_Right (1ULL<<13)
void padConfigureInput(int,int);
void padInitializeDefault(PadState *);
void hidInitializeTouchScreen(void);
void padUpdate(PadState *);
u64 padGetButtons(PadState *);
u64 padGetButtonsDown(PadState *);
u64 padGetButtonsUp(PadState *);
HidAnalogStickState padGetStickPos(PadState *,int);
void hidGetTouchScreenStates(HidTouchScreenState *,int);
/* Single-threaded real-JNI host tests. */
typedef int Mutex;
static inline void mutexInit(Mutex *m){*m=0;}
static inline void mutexLock(Mutex *m){(void)m;}
static inline void mutexUnlock(Mutex *m){(void)m;}
enum {BreakReason_Panic=0};
static inline void svcBreak(int reason,unsigned a,unsigned b){(void)reason;(void)a;(void)b;}
