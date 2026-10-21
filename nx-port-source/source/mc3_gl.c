/* SPDX-License-Identifier: MIT. Translate guest ATC uploads to RGBA8. */
#include "mc3_atc.h"
#include "util.h"
#include <GLES2/gl2.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
static void (*compressed_image)(GLenum,GLint,GLenum,GLsizei,GLsizei,GLint,GLsizei,const void *);
static void (*compressed_subimage)(GLenum,GLint,GLint,GLint,GLsizei,GLsizei,GLenum,GLsizei,const void *);
static int atc(GLenum f){return f==MC3_ATC_RGB||f==MC3_ATC_EXPLICIT_ALPHA||f==MC3_ATC_INTERPOLATED_ALPHA;}
static void image(GLenum t,GLint l,GLenum f,GLsizei w,GLsizei h,GLint border,GLsizei size,const void *data){
 if(!atc(f)){if(compressed_image)compressed_image(t,l,f,w,h,border,size,data);return;}
 uint8_t *pixels=NULL;int rc=mc3_atc_decode(f,w,h,size,data,&pixels);
 if(rc){debugPrintf("[mc3/atc] image rejected: format=%x %dx%d size=%d rc=%d\n",f,w,h,size,rc);return;}
 glTexImage2D(t,l,GL_RGBA,w,h,border,GL_RGBA,GL_UNSIGNED_BYTE,pixels);free(pixels);
}
static void subimage(GLenum t,GLint l,GLint x,GLint y,GLsizei w,GLsizei h,GLenum f,GLsizei size,const void *data){
 if(!atc(f)){if(compressed_subimage)compressed_subimage(t,l,x,y,w,h,f,size,data);return;}
 uint8_t *pixels=NULL;int rc=mc3_atc_decode(f,w,h,size,data,&pixels);
 if(rc){debugPrintf("[mc3/atc] subimage rejected: format=%x %dx%d size=%d rc=%d\n",f,w,h,size,rc);return;}
 glTexSubImage2D(t,l,x,y,w,h,GL_RGBA,GL_UNSIGNED_BYTE,pixels);free(pixels);
}
uintptr_t port_gl_wrap(const char *name,uintptr_t real){
 if(!strcmp(name,"glCompressedTexImage2D")){compressed_image=(void *)real;return(uintptr_t)image;}
 if(!strcmp(name,"glCompressedTexSubImage2D")){compressed_subimage=(void *)real;return(uintptr_t)subimage;}
 return 0;
}
