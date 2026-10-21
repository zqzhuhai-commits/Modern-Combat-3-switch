#pragma once
typedef unsigned GLenum;
typedef int GLint;
typedef int GLsizei;
#define GL_RGBA 0x1908
#define GL_UNSIGNED_BYTE 0x1401
void glTexImage2D(GLenum,GLint,GLint,GLsizei,GLsizei,GLint,GLenum,GLenum,const void *);
void glTexSubImage2D(GLenum,GLint,GLint,GLint,GLsizei,GLsizei,GLenum,GLenum,const void *);
