/* SPDX-License-Identifier: MIT. Standalone AArch32 MC3 runtime bring-up.
 * Boot order follows the verified EapRules mc3-native-arm main.cpp: JNI_OnLoad
 * -> GDRM unlock -> input exports -> GL2JNILib.init -> InAppBilling ->
 * MyVideoView -> GloftM3HM.Init -> nativeMC3Init -> getViewSettings -> resize.
 */
#include "config.h"
#include "mc3.h"
#include "jni.h"
#include "imports.h"
#include "dcr_path.h"
#include "dcr_time.h"
#include "dcr_sched.h"
#include "rt_applet.h"
#include "rt_boot.h"
#include "rt_window.h"
#include "rt_audout.h"
#include "selfproc.h"
#include "error.h"
#include "util.h"
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <switch.h>
Mc3Api g_mc3_api;
static so_module module;
static int running;
static EGLDisplay display;
static EGLSurface surface;
static EGLContext context;
extern EGLDisplay b_eglGetDisplay(EGLNativeDisplayType);
extern EGLBoolean b_eglInitialize(EGLDisplay,EGLint *,EGLint *);
extern EGLBoolean b_eglChooseConfig(EGLDisplay,const EGLint *,EGLConfig *,EGLint,EGLint *);
extern EGLContext b_eglCreateContext(EGLDisplay,EGLConfig,EGLContext,const EGLint *);
extern EGLSurface b_eglCreateWindowSurface(EGLDisplay,EGLConfig,void *,const EGLint *);
extern EGLBoolean b_eglMakeCurrent(EGLDisplay,EGLSurface,EGLSurface,EGLContext);
extern EGLBoolean b_eglSwapBuffers(EGLDisplay,EGLSurface);
void dcr_config_load(void);
static void stage(const char *s){debugPrintf("[mc3/stage] %s\n",s);mc3_jni_check(s);log_flush_ring();}
static uintptr_t lookup(void *m,const char *s){return so_try_find_addr_rx(m,s);}
static void note(const char *s){stage(s);}
static int verified(const char *path,uint64_t want_size,const char *want_sha256){
 FILE *f=fopen(path,"rb");if(!f)return 0;
 Sha256Context c;sha256ContextCreate(&c);unsigned char b[16384],h[32];size_t n,total=0;
 while((n=fread(b,1,sizeof b,f))){sha256ContextUpdate(&c,b,n);total+=n;}
 int error=ferror(f);fclose(f);sha256ContextGetHash(&c,h);
 char hex[65];for(int i=0;i<32;i++)snprintf(hex+2*i,3,"%02x",h[i]);
 debugPrintf("[mc3/library] %u bytes SHA256 %s\n",(unsigned)total,hex);
 return !error && total==want_size && !strcmp(hex,want_sha256);
}
/* Fast data check: full SHA256 of the library (9.7MB, tens of ms), size
 * checks of the two expansion packs (their zip CRCs are the engine's own
 * verification when it opens them). */
static int required_data(void){
 char p[768];
 snprintf(p,sizeof p,"%s/libModernCombat3.so",dcr_game_root());
 if(!verified(p,9766612,"99cd2fe38e4387c362c4435437e53d90c55ebea61e36695e49c8efdf1202d9b2")){
  snprintf(p,sizeof p,"%s/lib/armeabi-v7a/libModernCombat3.so",dcr_game_root());
  if(!verified(p,9766612,"99cd2fe38e4387c362c4435437e53d90c55ebea61e36695e49c8efdf1202d9b2")){
   stage("unsupported or missing original library");return -1;}
 }
 struct stat st;
 snprintf(p,sizeof p,"%s/" MC3_MAIN_OBB,dcr_game_root());
 if(stat(p,&st)||st.st_size!=1342717349){debugPrintf("[mc3/data] main.obb missing or wrong size\n");return -1;}
 snprintf(p,sizeof p,"%s/" MC3_PATCH_OBB,dcr_game_root());
 if(stat(p,&st)||st.st_size!=81507652){debugPrintf("[mc3/data] patch.obb missing or wrong size\n");return -1;}
 stage("original library and expansion packs present");return 0;
}
const char *port_path_fixup(const char *real,char *out,size_t cap){
 /* The engine's hardcoded Android paths map into the game root: the two
  * expansion packs read from /sdcard/Android/obb/<pkg>, saves and logs write
  * under /data/data/<pkg>. */
 static const char *kObb="/sdcard/Android/obb/" MC3_PKG;
 static const char *kData="/data/data/" MC3_PKG;
 static const char *kExt="/sdcard/Android/data/" MC3_PKG;
 const char *p;
 if((p=strstr(real,kObb)) && p[strlen(kObb)]=='/'){
  snprintf(out,cap,"%s%s",dcr_game_root(),p+strlen(kObb));return out;
 }
 if((p=strstr(real,kData)) && p[strlen(kData)]=='/'){
  snprintf(out,cap,"%s/data%s",dcr_game_root(),p+strlen(kData));return out;
 }
 if((p=strstr(real,kExt)) && p[strlen(kExt)]=='/'){
  snprintf(out,cap,"%s/data%s",dcr_game_root(),p+strlen(kExt));return out;
 }
 return real;
}
static int load_game(void){
 if(required_data())return -1;
 char path[768];snprintf(path,sizeof path,"%s/libModernCombat3.so",dcr_game_root());
 if(access(path,F_OK))snprintf(path,sizeof path,"%s/lib/armeabi-v7a/libModernCombat3.so",dcr_game_root());
 stage("load ELF");if(dcr_self_process()==INVALID_HANDLE){stage("process handle unavailable");return -1;}
 int rc=so_load(&module,path,NULL,PORT_SO_REGION_BYTES);if(rc){debugPrintf("[mc3] so_load=%d\n",rc);return -1;}
 if(so_relocate(&module)){stage("relocation failed");return -1;}
 int missing=so_resolve(&module,dcr_imports,dcr_imports_count,1);
 debugPrintf("[mc3] unresolved imports=%d\n",missing);if(missing)return -1;
 if(so_fix_kuser_helpers(&module)<0)return -1;
 so_finalize(&module);so_flush_caches(&module);
 for(int i=0;i<module.phnum;i++)if(module.phdr[i].p_type==PT_LOAD && (module.phdr[i].p_flags&PF_X)){
  uint32_t begin=(uint32_t)(uintptr_t)module.load_virtbase+module.phdr[i].p_vaddr;
  mc3_swp_region(begin,begin+module.phdr[i].p_memsz);break;
 }
 int n=mc3_bind(&g_mc3_api,lookup,&module,note);if(n){debugPrintf("[mc3] missing native exports=%d\n",n);return -1;}
 stage("ELF mapped, imports resolved, native exports bound");return 0;
}
static int graphics(void){
 stage("create EGL/GLES2 window");dcr_window_set_size(MC3_OUTPUT_WIDTH,MC3_OUTPUT_HEIGHT);
 display=b_eglGetDisplay(EGL_DEFAULT_DISPLAY);EGLint maj=0,min=0;
 if(!b_eglInitialize(display,&maj,&min))return -1;
 EGLint attrs[]={EGL_SURFACE_TYPE,EGL_WINDOW_BIT,EGL_RENDERABLE_TYPE,EGL_OPENGL_ES2_BIT,
 EGL_RED_SIZE,8,EGL_GREEN_SIZE,8,EGL_BLUE_SIZE,8,EGL_ALPHA_SIZE,8,EGL_DEPTH_SIZE,24,EGL_STENCIL_SIZE,8,EGL_NONE};
 EGLConfig config;EGLint count=0;
 if(!b_eglChooseConfig(display,attrs,&config,1,&count))return -1;
 if(!count){attrs[13]=16;attrs[15]=0;if(!b_eglChooseConfig(display,attrs,&config,1,&count)||!count)return -1;}
 const EGLint ca[]={EGL_CONTEXT_CLIENT_VERSION,2,EGL_NONE};
 context=b_eglCreateContext(display,config,EGL_NO_CONTEXT,ca);
 surface=b_eglCreateWindowSurface(display,config,NULL,NULL);
 if(!context||!surface||!b_eglMakeCurrent(display,surface,surface,context))return -1;
 debugPrintf("[mc3/gl] vendor=%s renderer=%s GLES=%s\n",glGetString(GL_VENDOR),glGetString(GL_RENDERER),glGetString(GL_VERSION));
 eglSwapInterval(display,1);glClearColor(0.015f,0.025f,0.04f,1);glClear(GL_COLOR_BUFFER_BIT);
 if(!b_eglSwapBuffers(display,surface))return -1;
 stage("EGL surface presented; calling original game next");return 0;
}
void port_focus_lost(void){if(running){mc3_input_release();mc3_audio_pause(1);stage("focus lost");}}
void port_focus_gained(void){if(running){mc3_audio_pause(0);stage("focus gained");}}
/* Entry-point calling conventions (softfp, same ABI as the runtime). */
typedef void (*v0_fn)(void);
typedef void (*v1e_fn)(void *,void *);
typedef void (*v2e_fn)(void *,void *,void *);
typedef void (*v4e_fn)(void *,void *,int,int);
typedef void (*v1e2_fn)(void *,void *,int,int);
static void call_v0(uintptr_t p){((v0_fn)p)();}
static void call_v1e(uintptr_t p,void *a){((v1e_fn)p)(a,NULL);}
static void call_v2e(uintptr_t p,void *a,void *b){((v2e_fn)p)(a,b,NULL);}
static void call_v4e(uintptr_t p,void *a,int x,int y){((v4e_fn)p)(a,NULL,x,y);}
static int run_game(void){
 stage("initialize JNI object model");jni_init();
 extern int g_so_trace_ctors;g_so_trace_ctors=1;
 stage("execute native constructors");so_execute_init_array(&module);so_free_temp(&module);
 if(graphics()){stage("EGL creation/presentation failed");return -1;}
 debugPrintf("[mc3/display] output=%dx%d fullscreen; native asset coordinates=%dx%d\n",
             MC3_OUTPUT_WIDTH,MC3_OUTPUT_HEIGHT,MC3_WIDTH,MC3_HEIGHT);
 stage("JNI/native bootstrap begins");
 void *gl_class=jni_class("com/gameloft/glf/GL2JNILib");
 void *game_class=jni_class(MC3_CLASS "GloftM3HM");
 void *game_thiz=jni_singleton(MC3_CLASS "GloftM3HM");
 if(!gl_class||!game_class||!game_thiz){stage("class lookup failed");return -1;}
 void *env=g_jni_env,*vm=g_jni_vm;
 int version=((int (*)(void *,void *))g_mc3_api.on_load)(vm,NULL);
 if(version!=0x00010001&&version!=0x00010002&&version!=0x00010004&&version!=0x00010006){
  debugPrintf("[mc3] JNI_OnLoad version=0x%08x\n",version);return -1;}
 stage("JNI_OnLoad returned; unlocking GDRM");
 mc3_unlock_drm(&module);
 mc3_input_init();
 call_v1e(g_mc3_api.gl_init,env,gl_class);
 if(g_mc3_api.billing_init)call_v2e(g_mc3_api.billing_init,env,game_class);
 if(g_mc3_api.video_init)call_v1e(g_mc3_api.video_init,env);
 if(g_mc3_api.game_init)call_v1e(g_mc3_api.game_init,env,game_thiz);
 if(g_mc3_api.game_mc3_init)call_v1e(g_mc3_api.game_mc3_init,env,game_class);
 if(g_mc3_api.view_settings)call_v0(g_mc3_api.view_settings);
 if(g_mc3_api.resize)call_v4e(g_mc3_api.resize,env,MC3_WIDTH,MC3_HEIGHT);
 stage("native bootstrap returned; entering frame loop");
 running=1;
 unsigned frames=0;
 while(appletMainLoop()&&!rt_exit_requested()){
  rt_applet_poll();if(!rt_focused()){svcSleepThread(50000000);continue;}
  if(mc3_input_poll())break;
  mc3_video_tick();
  call_v0(g_mc3_api.step);
  if(!b_eglSwapBuffers(display,surface)){stage("EGL swap failed");break;}
  if(++frames%300==0){debugPrintf("[mc3/frame] presented=%u JNI_objects=%d SWP=%u\n",
     frames,jni_live_objects(),mc3_swp_count());log_flush_ring();}
 }
 stage("native shutdown");mc3_input_release();mc3_audio_close();running=0;
 rt_applet_stop();eglMakeCurrent(display,EGL_NO_SURFACE,EGL_NO_SURFACE,EGL_NO_CONTEXT);
 eglDestroySurface(display,surface);eglDestroyContext(display,context);eglTerminate(display);
 return 0;
}
int main(int argc,char **argv){
 (void)argc;(void)argv;const char *root=dcr_game_root();mkdir(root,0777);
 log_init(root);log_console_open();log_console_show_text();
 debugPrintf("Modern Combat 3: Fallen Nation runtime / NSP bring-up / gameplay unverified\n");
 dcr_report_boot();if(chdir(root))debugPrintf("[mc3] root chdir failed\n");
 dcr_config_load();dcr_time_init();dcr_path_prepare_dirs();
 debugPrintf("A: attempt original game initialization. PLUS: exit.\n");log_flush_ring();
 padConfigureInput(1,HidNpadStyleSet_NpadStandard);PadState pad;padInitializeDefault(&pad);
 int start=0;while(appletMainLoop()){
  padUpdate(&pad);u64 down=padGetButtonsDown(&pad);
  if(down&HidNpadButton_Plus)break;
  if(down&HidNpadButton_A){start=1;break;}
  log_console_update();svcSleepThread(16000000);
 }
 if(!start)return 0;
 if(load_game()==0){dcr_sched_init();rt_applet_start();if(run_game()==0){log_flush_ring();return 0;}}
 stage("boot stopped; see debug.log / crash.log");
 while(appletMainLoop()){
  padUpdate(&pad);if(padGetButtonsDown(&pad)&HidNpadButton_Plus)break;
  if(!display)log_console_update();
  svcSleepThread(16000000);
 }
 log_flush_ring();return 1;
}
