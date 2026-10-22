/* Exercise the actual port handlers; only platform services are mocked. */
#include "mc3.h"
#include "jni.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
Mc3Api g_mc3_api;
void *g_jni_env=(void *)1;
static unsigned char written[64];
static unsigned written_size, written_rate;
static int written_channels, audio_result=10000,finished;
static JObj *allocated[128];static unsigned allocated_count;
static JObj *obj(void){JObj *p=calloc(1,sizeof *p);assert(p);assert(allocated_count<128);allocated[allocated_count++]=p;return p;}
JObj *jni_array(char elem,int len){JObj *p=obj();p->kind=JK_ARRAY;p->a.elem=elem;p->a.len=len;p->a.data=calloc(len?len:1,2);return p;}
JObj *jni_str(const char *s){JObj *p=obj();p->kind=JK_STRING;p->s.utf=strdup(s);return p;}
const char *jni_utf(const void *o){return o?((const JObj *)o)->s.utf:"";}
JObj *jni_singleton(const char *s){return jni_str(s);}
JClass *jni_class(const char *s){static JClass c;static JObj o;(void)s;c.obj=&o;return &c;}
const char *dcr_game_root(void){return "sdmc:/switch/mc3_nx";}
void debugPrintf(const char *s,...){(void)s;}
void log_flush_ring(void){}
void mc3_audio_pause(int p){(void)p;}
void mc3_audio_close(void){}
int mc3_audio_write(const unsigned char *b,unsigned n,unsigned r,int c){assert(n<=sizeof written);memcpy(written,b,n);written_size=n;written_rate=r;written_channels=c;return audio_result==10000?(int)n:audio_result;}
static void finish(void *env,void *cls){assert(env==g_jni_env);assert(cls==jni_class("video/MyVideoView")->obj);finished++;}
static int invoke(JObj *self,const char *cls,const char *name,const char *sig,const jvalue *a,jvalue *out){JClass c={0};JMethod m={0};snprintf(c.name,sizeof c.name,"%s",cls);m.cls=&c;snprintf(m.name,sizeof m.name,"%s",name);snprintf(m.sig,sizeof m.sig,"%s",sig);return port_jni_invoke(self,&m,a,out);}
int main(void){
 JObj track={0};jvalue a[6]={0},out;
 a[1]=jv_i(44100);a[2]=jv_i(4);assert(invoke(&track,"android/media/AudioTrack","<init>","(IIIIII)V",a,&out));assert(track.v[1]==1);
 JObj *samples=jni_array('S',5);short data[]={100,0x1234,-2,300,400};memcpy(samples->a.data,data,sizeof data);
 a[0]=jv_l(samples);a[1]=jv_i(1);a[2]=jv_i(2);
 assert(invoke(&track,"android/media/AudioTrack","write","([SII)I",a,&out));assert(out.i==2&&written_size==4&&written_rate==44100&&written_channels==1);assert(written[0]==0x34&&written[1]==0x12&&written[2]==0xfe&&written[3]==0xff);
 audio_result=-1;invoke(&track,"android/media/AudioTrack","write","([SII)I",a,&out);assert(out.i==-1);audio_result=10000;
 a[1]=jv_i(-1);invoke(&track,"android/media/AudioTrack","write","([SII)I",a,&out);assert(out.i==-2);
 a[1]=jv_i(4);a[2]=jv_i(2);invoke(&track,"android/media/AudioTrack","write","([SII)I",a,&out);assert(out.i==-2);
 a[1]=jv_i(48000);a[2]=jv_i(12);invoke(&track,"android/media/AudioTrack","<init>","(IIIIII)V",a,&out);assert(track.v[1]==2);
 JObj *bytes=jni_array('B',8);memcpy(bytes->a.data,"ABCDEFGH",8);a[0]=jv_l(bytes);a[1]=jv_i(4);a[2]=jv_i(4);invoke(&track,"android/media/AudioTrack","write","([BII)I",a,&out);assert(out.i==4&&!memcmp(written,"EFGH",4));
 const char *contexts[]={"android/content/Context","android/app/Activity",MC3_CLASS "GloftM3HM"};
 const char *services[]={"phone","window","input_method","audio","connectivity"};
 const char *service_classes[]={"android/telephony/TelephonyManager","android/view/WindowManager","android/view/inputmethod/InputMethodManager","android/media/AudioManager","android/net/ConnectivityManager"};
 for(unsigned i=0;i<3;i++)for(unsigned j=0;j<5;j++){
  a[0]=jv_l(jni_str(services[j]));
  assert(invoke(NULL,contexts[i],"getSystemService","(Ljava/lang/String;)Ljava/lang/Object;",a,&out));
  assert(out.l && !strcmp(jni_utf(out.l),service_classes[j]));
 }
 assert(invoke(NULL,"com/gameloft/glf/GL2JNILib","getDeviceLanguage","()Ljava/lang/String;",a,&out));
 assert(out.l && !strcmp(jni_utf(out.l),"en"));
 assert(invoke(NULL,"android/os/Process","setThreadPriority","(I)V",a,&out));
 assert(!invoke(NULL,"unrelated/Class","getSystemService","(Ljava/lang/String;)Ljava/lang/Object;",a,&out));
 g_mc3_api.video_finished=(uintptr_t)finish;
 const char *classes[]={"android/app/Activity",MC3_CLASS "GloftM3HM","video/MyVideoView"};
 const char *names[]={"startActivity","startActivityForResult","playMovie"};
 for(int i=0;i<3;i++){assert(invoke(NULL,classes[i],names[i],"()V",a,&out));for(int j=0;j<89;j++)mc3_video_tick();assert(finished==i);mc3_video_tick();assert(finished==i+1);mc3_video_tick();assert(finished==i+1);}
 assert(!invoke(NULL,"unrelated/Class","playMovie","()V",a,&out));
 for(unsigned i=0;i<allocated_count;i++){JObj *p=allocated[i];if(p->kind==JK_ARRAY)free(p->a.data);if(p->kind==JK_STRING)free(p->s.utf);free(p);}
 puts("JNI/audio/video bridge tests passed");return 0;
}
