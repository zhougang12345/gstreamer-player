//
// Created by zhougang on 2019/6/3.
//

#ifndef ANDROID_OGL_H
#define ANDROID_OGL_H
#include <gst/gst.h>
#include <gst/gl/gl.h>
#include <gst/gl/egl/gstgldisplay_egl.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <pthread.h>
#include <gst/player/gstplayer.h>
#include <gst/player/gstplayer-video-overlay-video-renderer.h>
#include <stdlib.h>
#include <assert.h>
#include <jmorecfg.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "stream.h"
typedef struct {
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    ANativeWindow * native_window;
    EGLint width;
    EGLint height;
    GList *list;
    gboolean isRunning;
}OGL;
void open_ogl(OGL* ogl);
void close_ogl(OGL* ogl);
#endif //ANDROID_OGL_H
