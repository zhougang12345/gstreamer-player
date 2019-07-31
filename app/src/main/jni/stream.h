//
// Created by zhougang on 2019/5/29.
//

#ifndef ANDROID_STREAM_H
#define ANDROID_STREAM_H
#include <string.h>
#include <stdint.h>
#include <jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <gst/gst.h>
#include <gst/gl/gl.h>
#include <gst/gl/egl/gstgldisplay_egl.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <pthread.h>
#include <gst/player/gstplayer.h>
#include <gst/player/gstplayer-video-overlay-video-renderer.h>
#include <stdlib.h>
#include <assert.h>
#include <jmorecfg.h>

#define SYNC_BUFFERS FALSE
typedef enum {
    GST_PLAY_FLAG_VIDEO             = (1 << 0),
    GST_PLAY_FLAG_AUDIO             = (1 << 1),
    GST_PLAY_FLAG_TEXT              = (1 << 2),
    GST_PLAY_FLAG_VIS               = (1 << 3),
    GST_PLAY_FLAG_SOFT_VOLUME       = (1 << 4),
    GST_PLAY_FLAG_NATIVE_AUDIO      = (1 << 5),
    GST_PLAY_FLAG_NATIVE_VIDEO      = (1 << 6),
    GST_PLAY_FLAG_DOWNLOAD          = (1 << 7),
    GST_PLAY_FLAG_BUFFERING         = (1 << 8),
    GST_PLAY_FLAG_DEINTERLACE       = (1 << 9),
    GST_PLAY_FLAG_SOFT_COLORBALANCE = (1 << 10),
    GST_PLAY_FLAG_FORCE_FILTERS     = (1 << 11),
} SpiceGstPlayFlags;
typedef struct {
    GstElement *pipeline;
    GstElement *vsink;
    gpointer *gst_display;
    gpointer *gl_context;
    GAsyncQueue *queue;
    GMutex queue_lock;
    GCond cond;
    gboolean flushing;
    GstMiniObject *popped_obj;
    GLuint textureId;
    gchar *uri;
    GstBuffer *current_buffer;
    unsigned char *texture;
    gint width;
    gint height;
}STREAM;
void create_pipeline(STREAM *stream);
gboolean handle_queued_objects (STREAM *stream);
#endif //ANDROID_STREAM_H
