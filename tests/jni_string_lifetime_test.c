/* Exercise the real JNIEnv table, including local-reference deletion. */
#include "jni.h"
#include "rt_cfg.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
const JMethodDef jni_method_defs[]={{NULL,NULL,NULL,NULL}};
const JFieldDef jni_field_defs[]={{NULL,NULL,NULL,0,NULL}};
const char *const jni_class_supers[][2]={{NULL,NULL}};
const char *const jni_missing_classes[]={NULL};
const RtConfig *rt_config(void){return NULL;}
const char *dcr_game_root(void){return "/nonexistent-mc3-test";}
void debugPrintf(const char *fmt,...){(void)fmt;}
int b_vsnprintf(char *dst,size_t cap,const char *fmt,va_list ap){return vsnprintf(dst,cap,fmt,ap);}
int main(void){
 jni_init();void **fn=*(void ***)g_jni_env;
 typedef void *(*new_fn)(void *,const char *);
 typedef const char *(*get_fn)(void *,void *,jboolean *);
 typedef void (*del_fn)(void *,void *);
 typedef void (*rel_fn)(void *,void *,const char *);
 new_fn make=(new_fn)fn[167];get_fn get=(get_fn)fn[169];
 del_fn del=(del_fn)fn[23];rel_fn release=(rel_fn)fn[170];
 const char *paths[]={"sdmc:/switch/mc3_nx/main.1120.com.gameloft.android.ANMP.GloftM3HM.obb", "sdmc:/switch/mc3_nx/patch.11428.com.gameloft.android.ANMP.GloftM3HM.obb", "en", ""};
 for(unsigned i=0;i<4;i++){
  void *s=make(g_jni_env,paths[i]);jboolean copied=0;
  const char *a=get(g_jni_env,s,&copied);
  const char *b=get(g_jni_env,s,NULL);
  del(g_jni_env,s);
  /* JNI character buffers remain valid until their own matching release,
   * even when the local String reference was already deleted. */
  assert(!strcmp(a,paths[i]) && !strcmp(b,paths[i]));assert(copied);
  release(g_jni_env,s,a);
  assert(!strcmp(b,paths[i]));release(g_jni_env,s,b);
 }
 puts("Real JNIEnv string-buffer lifetime tests passed");
}
