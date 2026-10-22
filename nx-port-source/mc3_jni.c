/* SPDX-License-Identifier: MIT. MC3-specific JNI; reference:
 * EapRules mc3-native-arm (jni/), whose answers were verified on hardware.
 * Exact class names are from the supplied native ELF. Unknown calls remain logged.
 */
#include "mc3.h"
#include "jni.h"
#include "rt_applet.h"
#include "dcr_path.h"
#include "util.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

extern int mc3_audio_write(const unsigned char *bytes, unsigned size,
                           unsigned rate, int channels);
extern void mc3_audio_pause(int pause);
extern void mc3_audio_close(void);

const JMethodDef jni_method_defs[]={{NULL,NULL,NULL,NULL}};
const JFieldDef jni_field_defs[]={
 {"android/content/Context","TELEPHONY_SERVICE",NULL,0,"phone"},
 {"android/os/Build","MODEL",NULL,0,"Nintendo Switch"},
 {"android/os/Build","MANUFACTURER",NULL,0,"Nintendo"},
 {"android/os/Build$VERSION","SDK_INT",NULL,10,NULL},
 {"android/os/Build$VERSION","RELEASE",NULL,0,"2.3.4"},
 {NULL,NULL,NULL,0,NULL}
};
const char *const jni_class_supers[][2]={
 {MC3_CLASS "GloftM3HM","android/app/Activity"},
 {MC3_CLASS "GLUtils/SUtils","android/app/Activity"},
 {"android/app/Activity","android/content/Context"},
 {"android/content/Context","java/lang/Object"},
 {MC3_CLASS "GLUtils/Device","java/lang/Object"},
 {MC3_CLASS "installer/GameInstaller","java/lang/Object"},
 {MC3_CLASS "installer/GDRMPolicy","java/lang/Object"},
 {MC3_CLASS "iab/InAppBilling","java/lang/Object"},
 {"com/gameloft/glf/GL2JNILib","java/lang/Object"},
 {"video/MyVideoView","java/lang/Object"},
 {NULL,NULL}
};
const char *const jni_missing_classes[]={NULL};
static unsigned unknown,reported;
static int is_mc3(const char *c){return !strncmp(c,MC3_CLASS,strlen(MC3_CLASS));}
static JObj *byte_string(const char *s){
 size_t n=strlen(s)+1;JObj *o=jni_array('B',(int)n);
 if(o)memcpy(o->a.data,s,n);
 return o;
}
static void set_dir(const JMethod *m,const jvalue *a,jvalue *out,const char *tail){
 char p[768];
 if(tail)snprintf(p,sizeof p,"%s%s",dcr_game_root(),tail);
 else snprintf(p,sizeof p,"%s",dcr_game_root());
 *out=jv_l(jni_str(p));
 (void)m;(void)a;
}
int port_jni_invoke(JObj *self,JMethod *m,const jvalue *a,jvalue *out){
 const char *c=m->cls->name,*n=m->name;
 if((!strcmp(c,"java/lang/System") && !strcmp(n,"currentTimeMillis")) || (is_mc3(c) && !strcmp(n,"GetCurrentTime"))){
  struct timespec ts;clock_gettime(CLOCK_REALTIME,&ts);*out=jv_j((int64_t)ts.tv_sec*1000+ts.tv_nsec/1000000);return 1;
 }
 if(!strcmp(c,"android/media/AudioTrack")){
  if(!strcmp(n,"<init>")){
   if(self){self->v[0]=a[1].i;self->v[1]=(a[2].i==12 || a[2].i==3)?2:1;}
   *out=jv_none();return 1;
  }
  if(!strcmp(n,"getMinBufferSize")){*out=jv_i(16384);return 1;}
  if(!strcmp(n,"write") && (!strcmp(m->sig,"([BII)I")||!strcmp(m->sig,"([SII)I"))){
   JObj *o=a[0].l;int off=a[1].i,len=a[2].i;
   if(!o || o->kind!=JK_ARRAY || off<0 || len<0 || off>o->a.len || len>o->a.len-off){*out=jv_i(-2);return 1;}
   unsigned rate=self&&self->v[0]?(unsigned)self->v[0]:44100;
   int channels=self&&self->v[1]?(int)self->v[1]:2;
   if(!strcmp(m->sig,"([SII)I")){
    /* 16-bit samples in a short[]; convert into the byte stream. */
    size_t bytes=(size_t)len*2;
    unsigned char *buf=(unsigned char *)malloc(bytes?bytes:1);
    if(!buf){*out=jv_i(-2);return 1;}
    const short *s=(const short *)o->a.data+off;
    for(int i=0;i<len;i++){buf[2*i]=(unsigned char)s[i];buf[2*i+1]=(unsigned char)((unsigned)s[i]>>8);}
    int r=mc3_audio_write(buf,(unsigned)bytes,rate,channels);
    free(buf);*out=jv_i(r<0?r:r/2);return 1;
   }
   *out=jv_i(mc3_audio_write((unsigned char *)o->a.data+off,len,rate,channels));return 1;
  }
  if(!strcmp(n,"play")){mc3_audio_pause(0);*out=jv_none();return 1;}
  if(!strcmp(n,"pause")||!strcmp(n,"stop")||!strcmp(n,"release")||!strcmp(n,"flush")){mc3_audio_pause(1);*out=jv_none();return 1;}
 }
 if(!strcmp(c,"android/telephony/TelephonyManager")){
  if(!strcmp(n,"getDeviceId")||!strcmp(n,"getSubscriberId")||!strcmp(n,"getLine1Number")||!strcmp(n,"getNetworkOperatorName")||!strcmp(n,"getNetworkOperator")){
   *out=jv_l(jni_str(""));return 1;
  }
  if(!strcmp(n,"getNetworkCountryIso")||!strcmp(n,"getSimCountryIso")){*out=jv_l(jni_str("us"));return 1;}
  if(!strcmp(n,"getSimState")){*out=jv_i(1);return 1;}
  if(!strcmp(n,"isNetworkRoaming")){*out=jv_z(0);return 1;}
 }
 /* Android Activity callbacks can be looked up on the base class, before
  * the game-class filter below. Briefings use this path. */
 if((!strcmp(c,"android/app/Activity") || is_mc3(c)) &&
    (!strcmp(n,"startActivity") || !strcmp(n,"startActivityForResult"))){
  mc3_video_play();*out=jv_none();return 1;
 }
 if(!strcmp(c,"video/MyVideoView") && !strcmp(n,"playMovie")){
  mc3_video_play();*out=jv_none();return 1;
 }
 /* Framework methods may be looked up on Context itself. Handle them
  * before filtering game classes; otherwise the service answer is unreachable. */
 if((!strcmp(c,"android/content/Context") || !strcmp(c,"android/app/Activity") || is_mc3(c)) && !strcmp(n,"getSystemService") && !strcmp(m->sig,"(Ljava/lang/String;)Ljava/lang/Object;")){
  /* Never answer null: the engine dereferences the service without a check.
   * window/input_method are the two EapRules answers the MC3 engine stores. */
  const char *svc=a[0].l?jni_utf(a[0].l):"";
  if(!strcmp(svc,"phone")||!strcmp(svc,"telephony"))*out=jv_l(jni_singleton("android/telephony/TelephonyManager"));
  else if(!strcmp(svc,"window"))*out=jv_l(jni_singleton("android/view/WindowManager"));
  else if(!strcmp(svc,"input_method")||!strcmp(svc,"inputmethod"))*out=jv_l(jni_singleton("android/view/inputmethod/InputMethodManager"));
  else if(!strcmp(svc,"wifi"))*out=jv_l(jni_singleton("android/net/wifi/WifiManager"));
  else if(!strcmp(svc,"audio"))*out=jv_l(jni_singleton("android/media/AudioManager"));
  else if(!strcmp(svc,"power"))*out=jv_l(jni_singleton("android/os/PowerManager"));
  else if(!strcmp(svc,"connectivity"))*out=jv_l(jni_singleton("android/net/ConnectivityManager"));
  else *out=jv_l(jni_singleton("android/content/Context"));
  return 1;
 }
 /* The first frame asks GL2JNILib for a Java String, then consumes its
  * UTF chars in native code. An unhandled call returns NULL and crashes.
  * Keep the existing English resource language until localization is supported. */
 if(!strcmp(c,"com/gameloft/glf/GL2JNILib") &&
    !strcmp(n,"getDeviceLanguage") && !strcmp(m->sig,"()Ljava/lang/String;")){
  *out=jv_l(jni_str("en"));return 1;
 }
 if(!strcmp(c,"android/os/Process") && !strcmp(n,"setThreadPriority") &&
    !strcmp(m->sig,"(I)V")){
  /* Android priority values cannot be applied as Horizon priorities. */
  *out=jv_none();return 1;
 }
 /* The game's own classes and the Android glue it reaches for. */
 if(!is_mc3(c) && strncmp(c,"com/gameloft/glf/",17) && strcmp(c,"video/MyVideoView") && strcmp(c,"java/io/File") && strcmp(c,"android/content/Intent"))return 0;
 /* Xperia Play input path: this is what makes physical controls work. */
 if(!strcmp(n,"isXperiaPlay")){*out=jv_z(1);return 1;}
 /* The device is opened; the engine gates the physical-key path on this. */
 if(!strcmp(n,"isDeviceOpened")){*out=jv_z(1);return 1;}
 /* The engine keeps device-class queries in one obfuscated family. */
 if(!strcmp(n,"isHTCDevice")||!strcmp(n,"isSamsung_I9250")||
    !strcmp(n,"isSamsung_I510")||!strcmp(n,"isSamsung_P7100")||!strcmp(n,"isAsus_TF201")||
    !strcmp(n,"isHTC_Nexus9")||!strcmp(n,"isHoneycombDevice")||!strcmp(n,"isIceCreamSandwich")||
    !strcmp(n,"mustSwapButtons")||!strcmp(n,"isFirstRunOfTheDay")||!strcmp(n,"useGeneration0")||
    !strcmp(n,"useGeneration1")||!strcmp(n,"hasGyroscope")||!strcmp(n,"isGyroSensibilityHigh")||
    !strcmp(n,"isGyroSensibilityLow")||!strcmp(n,"isGyroPendingToFix")||!strcmp(n,"ignoreGyroLowInc")||
    !strcmp(n,"isGyroInvertedX")||!strcmp(n,"isInvertedPitchYaw")||!strcmp(n,"isSwitchRollToYaw")){
  *out=jv_z(0);return 1;
 }
 if(!strcmp(n,"useGeneration2")){*out=jv_z(1);return 1;}
 if(!strcmp(n,"enableGyroscope")||!strcmp(n,"enableAccelerometer")||!strcmp(n,"setCalibratingGyro")){
  *out=jv_none();return 1;
 }
 /* Loading-overlay callbacks and friends. EapRules registers every one of
  * these on the GloftM3HM class; the engine drives its boot state machine
  * through them, so answer instead of logging. */
 if(!strcmp(n,"showloading")||!strcmp(n,"showDisclaimer")||!strcmp(n,"showCantGoBackPopup")||
    !strcmp(n,"showParentButton")||!strcmp(n,"ShowAlert")||!strcmp(n,"openBrowser")||
    !strcmp(n,"sendURLRequest")||!strcmp(n,"openGLLive")||!strcmp(n,"openIGP")){*out=jv_none();return 1;}
 if(!strcmp(n,"hideloading")||!strcmp(n,"hidelogoscreen")||!strcmp(n,"hideParentButton")||
    !strcmp(n,"deleteOldData")||!strcmp(n,"setViewSettings")||!strcmp(n,"setCurrentContext")){*out=jv_none();return 1;}
 if(!strcmp(n,"hasServerResponse")){*out=jv_z(0);return 1;}
 if(!strcmp(n,"getServerResponse")){*out=jv_l(jni_str(""));return 1;}
 if(!strcmp(n,"IsWifiEnabled")||!strcmp(n,"IsWifiOr3GEnabled")){*out=jv_i(0);return 1;}
 if(!strcmp(n,"CheckOffensive")){*out=jv_z(0);return 1;}
 if(!strcmp(n,"getProcessorNum")){*out=jv_i(3);return 1;}
 if(!strcmp(n,"getDeviceName")){*out=jv_l(jni_str("Xperia Play"));return 1;}
 if(!strcmp(n,"getDeviceFW")){*out=jv_l(jni_str("4.1.2"));return 1;}
 if(!strcmp(n,"getUserAgent")){*out=jv_l(jni_str("Mozilla/5.0 (Linux; Android 4.1.2)"));return 1;}
 if(!strcmp(n,"Exit") || !strcmp(n,"sendAppToBackground")){debugPrintf("[mc3] original game requested exit\n");log_flush_ring();exit(0);}
 if(!strcmp(n,"getZipName")){set_dir(m,a,out,"/" MC3_MAIN_OBB);return 1;}
 if(!strcmp(n,"getPatchFileLocation")){set_dir(m,a,out,"/" MC3_PATCH_OBB);return 1;}
 if(!strcmp(n,"getSaveFolder")){set_dir(m,a,out,"/data");return 1;}
 if(!strcmp(n,"getSDFolder")||!strcmp(n,"getFilesDir")){set_dir(m,a,out,NULL);return 1;}
 if(!strcmp(n,"getPath")||!strcmp(n,"getAbsolutePath")){*out=jv_l(jni_str("/sdcard/Android/obb/" MC3_PKG "/"));return 1;}
 if(!strcmp(n,"getPackage")){*out=jv_l(jni_str(MC3_PKG));return 1;}
 if(!strcmp(n,"getPackageName")){*out=jv_l(jni_str(MC3_PKG));return 1;}
 if(!strcmp(n,"getPreferenceString") && !strcmp(m->sig,"(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;")){
  const char *fb=a[1].l?jni_utf(a[1].l):"";
  *out=jv_l(jni_str(fb?fb:""));return 1;
 }
 if(!strcmp(n,"getResource") && !strcmp(m->sig,"(Ljava/lang/String;)[B")){
  /* The APK's res/ is not part of a port; an empty array is the honest answer. */
  *out=jv_l(byte_string(""));return 1;
 }
 if(!strcmp(n,"setupPaths")){*out=jv_none();return 1;}
 if(!strcmp(n,"getData")){*out=jv_l(NULL);return 1;}
 if(!strcmp(c,"android/content/Intent") && !strcmp(n,"<init>")){*out=jv_none();return 1;}
 if(!strcmp(c,"android/content/Intent") && (!strcmp(n,"setData")||!strcmp(n,"setClass")||!strcmp(n,"setAction")||
    !strcmp(n,"putExtra")||!strcmp(n,"setFlags")||!strcmp(n,"addFlags")||!strcmp(n,"setType"))){*out=jv_l(jni_singleton("android/content/Intent"));return 1;}
 if(!strcmp(n,"getContext")||!strcmp(n,"getInstance")){*out=jv_l(jni_singleton(MC3_CLASS "GloftM3HM"));return 1;}
 if(!strcmp(n,"getResources")){*out=jv_l(jni_singleton("android/content/res/Resources"));return 1;}
 if(!strcmp(n,"getAssets")){*out=jv_l(jni_singleton("android/content/res/AssetManager"));return 1;}
 if(!strcmp(n,"getCountry")||!strcmp(n,"getNetworkCountryIso")||!strcmp(n,"getSimCountryIso")){*out=jv_l(jni_str("US"));return 1;}
 if(!strcmp(n,"getLanguage")){*out=jv_l(jni_str("en"));return 1;}
 if(!strcmp(n,"getPhoneModel")||!strcmp(n,"getDevice")){*out=jv_l(jni_str("Nintendo_Switch"));return 1;}
 if(!strcmp(n,"getCarrier")){*out=jv_l(jni_str("Nintendo"));return 1;}
 if(!strcmp(n,"getWidth")){*out=jv_i(MC3_WIDTH);return 1;}
 if(!strcmp(n,"getHeight")){*out=jv_i(MC3_HEIGHT);return 1;}
 if(!strcmp(n,"getManufacture")){*out=jv_i(4);return 1;}
 if(!strcmp(n,"GetPhoneLanguage")){*out=jv_i(0);return 1;}
 if(!strcmp(n,"isRoaming")||!strcmp(n,"getIsRoaming")||!strcmp(n,"IsWifiEnable")||!strcmp(n,"isWifiEnabled")){*out=jv_i(0);return 1;}
 if(!strcmp(n,"isNeedGcOnFrame")){*out=jv_z(0);return 1;}
 if(!strcmp(n,"getUniqueCode")||!strcmp(n,"createUniqueCode")){*out=jv_i(0);return 1;}
 if(!strcmp(n,"getIMEI")||!strcmp(n,"getHostName")||!strcmp(n,"getDemoCode")){*out=jv_l(jni_str(""));return 1;}
 if(!strcmp(n,"InitDeviceValues")||!strcmp(n,"init")||!strcmp(n,"destroy")||!strcmp(n,"openBrowser")||
    !strcmp(n,"notifyTrophy")||!strcmp(n,"sendTrackingInfo")||!strcmp(n,"nativeStart")||!strcmp(n,"setResult")){
  *out=jv_none();return 1;
 }
 /* Do not fabricate success for unimplemented calls. */
 if(unknown++<128)debugPrintf("[mc3/jni] unimplemented %s.%s%s\n",c,n,m->sig);
 return 0;
}
void mc3_jni_check(const char *stage){if(unknown!=reported){debugPrintf("[mc3/jni] stage=%s unimplemented calls=%u\n",stage,unknown);reported=unknown;}}

