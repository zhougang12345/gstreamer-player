//
// Created by zhougang on 2019/6/3.
//

#include "ogl.h"

void open_ogl(OGL* ogl){
    gint screen_num = 0;
    gulong black_pixel = 0;
    EGLBoolean result;
    EGLint num_config;

    EGLNativeWindowType window_handle = ogl->native_window;

    static const EGLint attribute_list[] = {
            EGL_DEPTH_SIZE, 16,
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
            EGL_NONE
    };

    static const EGLint context_attributes[] = {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
    };

    EGLConfig config;

    /* get an EGL display connection */
    ogl->display = eglGetDisplay (EGL_DEFAULT_DISPLAY);
    assert (ogl->display != EGL_NO_DISPLAY);

    /* initialize the EGL display connection */
    result = eglInitialize (ogl->display, NULL, NULL);
    assert (EGL_FALSE != result);

    result = eglChooseConfig (ogl->display, attribute_list, &config, 1, &num_config);

    /* create an EGL rendering context */
    ogl->context = eglCreateContext (ogl->display, config, EGL_NO_CONTEXT,context_attributes);
    assert ( ogl->context != EGL_NO_CONTEXT);

    ogl->surface = eglCreateWindowSurface (ogl->display, config, window_handle, NULL);
    assert (ogl->surface != EGL_NO_SURFACE);

    /* connect the context to the surface */
    result = eglMakeCurrent (ogl->display, ogl->surface, ogl->surface, ogl->context);
    assert (EGL_FALSE != result);

    eglQuerySurface(ogl->display, ogl->surface, EGL_WIDTH, &ogl->width);
    eglQuerySurface(ogl->display, ogl->surface, EGL_HEIGHT, &ogl->height);
    ogl->isRunning = TRUE;
    ogl->list = NULL;
}

void close_ogl(OGL* ogl){

    if (ogl->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(ogl->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (ogl->context != EGL_NO_CONTEXT) {
            eglDestroyContext(ogl->display,ogl->context);
        }
        if (ogl->surface != EGL_NO_SURFACE) {
            eglDestroySurface(ogl->display, ogl->surface);
        }
        eglTerminate(ogl->display);
    }
    ogl->display = EGL_NO_DISPLAY;
    ogl->surface = EGL_NO_SURFACE;
    ogl->context = EGL_NO_CONTEXT;
}

