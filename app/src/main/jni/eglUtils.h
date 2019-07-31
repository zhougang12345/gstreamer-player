//
// Created by zhougang on 2019/5/24.
//

#ifndef ANDROID_EGLUTILS_H
#define ANDROID_EGLUTILS_H

#include <GLES2/gl2.h>
#include <GLES/gl.h>

#include <GLES/glext.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>
typedef struct {
    GLuint program;
    int vertexShaderID;
    int fragmentShaderID;
    GLuint aPositionHandle;
    GLuint fPositionHandle;
    GLuint sTexture;
    GLuint primary_texId;
    GLuint cursor_texId;
}RENDER;
enum{
    sample2D,
    externalOES
};

void openRenderProg(RENDER *render,int type);
void genTexture(GLuint *textureId);
void blindTexture(GLuint textureId,void* texture,GLsizei texWidth,GLsizei texHeight);
void setVertex(GLint x,GLint y,GLint width,GLint height,GLint max_width,GLint max_height);
void draw(RENDER *render,GLuint textureId,int type);
void closeRenderProg(RENDER *render);
#endif //ANDROID_EGLUTILS_H
