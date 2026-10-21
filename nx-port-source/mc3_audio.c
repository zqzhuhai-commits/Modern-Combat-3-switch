/* SPDX-License-Identifier: MIT. AudioTrack PCM -> libnx audout. */
#include "mc3.h"
#include "rt_audout.h"
#include <stdint.h>
#include <string.h>
#include <switch.h>
static Mutex lock;
static int16_t block[RT_AUDOUT_FRAMES*2];
static unsigned used,phase;
static int paused;
int mc3_audio_write(const unsigned char *bytes,unsigned size,unsigned rate,int channels){
 if(!bytes || !rate || (channels!=1 && channels!=2) || size%(2u*channels))return -1;
 if(rate!=44100 && rate!=48000)return -1;
 mutexLock(&lock);
 if(paused){mutexUnlock(&lock);return (int)size;}
 if(rt_audout_open()!=0){mutexUnlock(&lock);return -1;}
 for(unsigned i=0;i<size;i+=2u*channels){
  int16_t l=(int16_t)((unsigned)bytes[i]|((unsigned)bytes[i+1]<<8));
  int16_t r=channels==2?(int16_t)((unsigned)bytes[i+2]|((unsigned)bytes[i+3]<<8)):l;
  phase+=rt_audout_rate();
  while(phase>=rate){
   phase-=rate;block[used*2]=l;block[used*2+1]=r;
   if(++used==RT_AUDOUT_FRAMES){
    used=0;
    if(rt_audout_submit(block)){mutexUnlock(&lock);return -1;}
   }
  }
 }
 mutexUnlock(&lock);return (int)size;
}
void mc3_audio_pause(int pause){
 rt_audout_cancel(pause);
 mutexLock(&lock);paused=pause;used=phase=0;mutexUnlock(&lock);
}
void mc3_audio_close(void){mc3_audio_pause(1);rt_audout_close();}
