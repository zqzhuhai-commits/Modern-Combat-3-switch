#include "mc3_atc.h"
#include <GLES2/gl2.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
uintptr_t port_gl_wrap(const char *,uintptr_t);
static int uploaded,forwarded;static uint8_t first[4];
void debugPrintf(const char *f,...){(void)f;}
void glTexImage2D(GLenum t,GLint l,GLint f,GLsizei w,GLsizei h,GLint b,GLenum fmt,GLenum type,const void *p){
 (void)t;(void)l;(void)w;(void)h;(void)b;
 assert(f==GL_RGBA&&fmt==GL_RGBA&&type==GL_UNSIGNED_BYTE);memcpy(first,p,4);uploaded++;
}
void glTexSubImage2D(GLenum t,GLint l,GLint x,GLint y,GLsizei w,GLsizei h,GLenum fmt,GLenum type,const void *p){
 (void)t;(void)l;(void)x;(void)y;(void)w;(void)h;
 assert(fmt==GL_RGBA&&type==GL_UNSIGNED_BYTE);memcpy(first,p,4);uploaded++;
}
static void fallback(GLenum t,GLint l,GLenum f,GLsizei w,GLsizei h,GLint b,GLsizei s,const void *p){
 (void)t;(void)l;(void)f;(void)w;(void)h;(void)b;(void)s;(void)p;forwarded++;
}
int main(void){
 uint8_t rgb[8]={0,0x7c,0x1f,0,0,0,0,0},*out=NULL;
 assert(!mc3_atc_decode(MC3_ATC_RGB,3,3,8,rgb,&out));
 for(int i=0;i<9;i++)assert(out[i*4]==255&&out[i*4+1]==0&&out[i*4+2]==0&&out[i*4+3]==255);
 free(out);out=NULL;
 assert(mc3_atc_decode(MC3_ATC_RGB,4,4,7,rgb,&out)<0&&out==NULL);
 assert(mc3_atc_decode(MC3_ATC_RGB,0,4,8,rgb,&out)<0);
 assert(mc3_atc_decode(0,4,4,8,rgb,&out)<0);
 typedef void(*Upload)(GLenum,GLint,GLenum,GLsizei,GLsizei,GLint,GLsizei,const void*);
 Upload image=(Upload)port_gl_wrap("glCompressedTexImage2D",(uintptr_t)fallback);
 image(0,0,MC3_ATC_RGB,4,4,0,8,rgb);assert(uploaded==1&&first[0]==255&&first[3]==255);
 image(0,0,0x83f0,4,4,0,8,rgb);assert(forwarded==1&&uploaded==1);
 image(0,0,MC3_ATC_RGB,4,4,0,7,rgb);assert(uploaded==1);
 uint8_t alpha[16]={0};memcpy(alpha+8,rgb,8);
 image(0,0,MC3_ATC_EXPLICIT_ALPHA,4,4,0,16,alpha);assert(uploaded==2&&first[3]==0);
 alpha[0]=255;
 image(0,0,MC3_ATC_INTERPOLATED_ALPHA,4,4,0,16,alpha);assert(uploaded==3&&first[3]==255);
 typedef void(*Sub)(GLenum,GLint,GLint,GLint,GLsizei,GLsizei,GLenum,GLsizei,const void*);
 Sub sub=(Sub)port_gl_wrap("glCompressedTexSubImage2D",0);
 sub(0,0,1,1,3,3,MC3_ATC_RGB,8,rgb);assert(uploaded==4&&first[0]==255);
 assert(!port_gl_wrap("glDrawArrays",123));return 0;
}
