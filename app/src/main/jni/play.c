//
// Created by zhougang on 5/17/19.
//

#include <string.h>
#include <stdint.h>
#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/bitmap.h>
#include "eglUtils.h"
#include "stream.h"
#include "ogl.h"
#include "draw.h"
#include <android/log.h>
#define LOGI(level, ...) __android_log_print(ANDROID_LOG_INFO, "NATIVE_LOG", __VA_ARGS__)
STREAM _stream1,_stream2,_stream3,_stream4,_stream5;
STREAM * stream1 = &_stream1;
STREAM * stream2 = &_stream2;
STREAM * stream3 = &_stream3;
STREAM * stream4 = &_stream4;
STREAM * stream5 = &_stream5;
void* src = NULL;
int bitmapWidth,bitmapHeight;
static OGL _ogl,*ogl = &_ogl;
static void streamInit(STREAM *stream){
    g_mutex_init (&stream->queue_lock);
    g_cond_init (&stream->cond);
    stream->current_buffer = NULL;
    stream->queue = g_async_queue_new_full ((GDestroyNotify) gst_mini_object_unref);
    stream->gst_display = (gpointer)gst_gl_display_egl_new_with_egl_display (ogl->display);
    stream->gl_context = (gpointer)gst_gl_context_new_wrapped (GST_GL_DISPLAY (stream->gst_display),
                                        (guintptr) ogl->context, GST_GL_PLATFORM_EGL, GST_GL_API_GLES2);
}
gpointer
render_func (gpointer data)
{

    open_ogl(ogl);
    RENDER * render = g_new0(RENDER,1);
    RENDER * render0 = g_new0(RENDER,1);
    genTexture(&render0->primary_texId);
    openRenderProg(render,externalOES);
    openRenderProg(render0,sample2D);


    streamInit(stream2);
    stream1->width = 1920;
    stream1->height = 1080;
    stream3->width = 960;
    stream3->height = 400;
    stream4->width = 1920;
    stream4->height = 1080;
    stream5->width = 960;
    stream5->height = 400;

    streamInit(stream1);
    streamInit(stream3);
    streamInit(stream4);
    streamInit(stream5);
    if (stream1->gst_display == stream2->gst_display){
        LOGI(1,"the same gst display");
    } else {
        LOGI(1,"not the same gst display");
    }
    if (stream1->gl_context == stream2->gl_context){
        LOGI(1,"not the same gl context");
    }
    stream2->width = 960;
    stream2->height = 400;
    unsigned char* x = g_new0(unsigned char,100 * 200 * 4);
    memset(x,0,100 * 200 * 4 * sizeof(unsigned char));
    LOGI(1,"x %hhd",x[100 * 200 * 4]);
    ogl->list = g_list_append(ogl->list,1);
    ogl->list = g_list_append(ogl->list,2);
    ogl->list = g_list_append(ogl->list,3);
    ogl->list = g_list_remove(ogl->list,2);
    ogl->list = g_list_remove(ogl->list,2);
    GList *list1= ogl->list;

    while (list1){
        LOGI(1,"list %d",list1->data);
        list1 = list1->next;
    }
    do {
        glClearColor(0.0f,0.0f,0.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        handle_queued_objects (stream1);
        handle_queued_objects (stream2);
        handle_queued_objects (stream3);
        handle_queued_objects (stream4);
        if (render0->primary_texId){

            blindTexture(render0->primary_texId,src,bitmapWidth,bitmapHeight);
            setVertex(0, 0, ogl->width, ogl->height, ogl->width, ogl->height);
            draw(render0,render0->primary_texId,sample2D);
        }

        if (stream1->current_buffer) {
            GstMemory *mem1 = gst_buffer_peek_memory(stream1->current_buffer, 0);
            setVertex(0, 200, 200, 200, ogl->width, ogl->height);
            draw(render,((GstGLMemory *) mem1)->tex_id,externalOES);
        }
        if (stream2->current_buffer) {
            GstMemory *mem2 = gst_buffer_peek_memory(stream2->current_buffer, 0);
            setVertex(250, 200, 200, 200, ogl->width, ogl->height);
            draw(render,((GstGLMemory *) mem2)->tex_id,externalOES);
        }
        if (stream3->current_buffer) {
            GstMemory *mem3 = gst_buffer_peek_memory(stream3->current_buffer, 0);
            setVertex(500, 200, 200, 200, ogl->width, ogl->height);
            draw(render,((GstGLMemory *) mem3)->tex_id,externalOES);
        }
        if (stream4->current_buffer) {
            GstMemory *mem4 = gst_buffer_peek_memory(stream4->current_buffer, 0);
            setVertex(750, 200, 200, 200, ogl->width, ogl->height);
            draw(render,((GstGLMemory *) mem4)->tex_id,externalOES);
        }

        eglSwapBuffers(ogl->display,ogl->surface);
        g_usleep (10000);
    } while (ogl->isRunning == TRUE);
    closeRenderProg(render);
    closeRenderProg(render0);
    close_ogl(ogl);
    return NULL;
}


JNIEXPORT
void JNICALL Java_org_freedesktop_gstreamer_MainActivity_setSurface(JNIEnv *env,jclass clazz,jobject surface){

    ANativeWindow *new_native_window = ANativeWindow_fromSurface(env, surface);
    ogl->native_window = new_native_window;
    GThread *rthread;
    if (!(rthread = g_thread_new("render", (GThreadFunc)render_func,NULL)))
    {
       LOGI(1,"thread create failed!");
    }
}
JNIEXPORT
void JNICALL Java_org_freedesktop_gstreamer_MainActivity_destroySurface(JNIEnv *env,jclass clazz){

    if (ogl->native_window){

        ANativeWindow_release(ogl->native_window);
    }
    ogl->isRunning = FALSE;
}

JNIEXPORT
void JNICALL Java_org_freedesktop_gstreamer_MainActivity_setUri(JNIEnv *env,jclass clazz,jstring uri1,jstring uri2){

    stream1->uri = (*env)->GetStringUTFChars(env, uri2, NULL);
    stream2->uri = (*env)->GetStringUTFChars(env, uri1, NULL);
    stream3->uri = (*env)->GetStringUTFChars(env, uri1, NULL);
    stream4->uri = (*env)->GetStringUTFChars(env, uri2, NULL);
//    stream5->uri = (*env)->GetStringUTFChars(env, uri1, NULL);

//    LOGI(1,"finish testTimer!");
    create_pipeline(stream1);
    create_pipeline(stream2);
    create_pipeline(stream3);
    create_pipeline(stream4);
//    create_pipeline(stream5);
}
JNIEXPORT
void JNICALL Java_org_freedesktop_gstreamer_MainActivity_setBitmap(JNIEnv *env,
                                                                    jclass clazz,
                                                                    jobject bitmap
                                                                    ) {


    AndroidBitmap_lockPixels(env, bitmap, (void**)&src);
    AndroidBitmapInfo srcInfo;
    AndroidBitmap_getInfo(env, bitmap, &srcInfo);
    bitmapWidth = srcInfo.width;
    bitmapHeight = srcInfo.height;
    //AndroidBitmap_unlockPixels(env, bitmap);
}
