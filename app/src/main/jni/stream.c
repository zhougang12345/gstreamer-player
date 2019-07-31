//
// Created by zhougang on 2019/5/29.
//

#include "stream.h"


static gboolean
queue_object (STREAM *stream,GstMiniObject * obj, gboolean synchronous)
{
    gboolean res = TRUE;
    g_mutex_lock (&stream->queue_lock);
    if (stream->flushing) {
        gst_mini_object_unref (obj);
        res = FALSE;
        goto beach;
    }

    g_async_queue_push (stream->queue, obj);

    if (synchronous) {
        /* Waiting for object to be handled */
        do {
            g_cond_wait (&stream->cond, &stream->queue_lock);
        } while (!stream->flushing && stream->popped_obj != obj);
    }

    beach:
    g_mutex_unlock (&stream->queue_lock);
    return res;
}

static void
flush_start (STREAM *stream)
{
    GstMiniObject *object = NULL;

    g_mutex_lock (&stream->queue_lock);
    stream->flushing = TRUE;
    g_cond_broadcast (&stream->cond);
    g_mutex_unlock (&stream->queue_lock);

    while ((object = g_async_queue_try_pop (stream->queue))) {
        gst_mini_object_unref (object);
    }
    g_mutex_lock (&stream->queue_lock);
    stream->popped_obj = NULL;
    g_mutex_unlock (&stream->queue_lock);
}


static void
flush_stop (STREAM *stream)
{
    GstMiniObject *object = NULL;
    g_mutex_lock (&stream->queue_lock);
    while ((object = GST_MINI_OBJECT_CAST (g_async_queue_try_pop(stream->queue)))) {
        gst_mini_object_unref (object);
    }
    stream->popped_obj = NULL;
    stream->flushing = FALSE;
    g_mutex_unlock (&stream->queue_lock);
}

static GstPadProbeReturn
events_cb (GstPad * pad, GstPadProbeInfo * probe_info, gpointer user_data)
{
    STREAM *stream = (STREAM*)user_data;
    GstEvent *event = GST_PAD_PROBE_INFO_EVENT (probe_info);

    switch (GST_EVENT_TYPE (event)) {

        case GST_EVENT_FLUSH_START:
            flush_start (stream);
            break;
        case GST_EVENT_FLUSH_STOP:
            flush_stop (stream);
            break;
        case GST_EVENT_EOS:
            queue_object (stream, GST_MINI_OBJECT_CAST (gst_event_ref (event)), FALSE);
            break;
        default:
            break;
    }
    return GST_PAD_PROBE_OK;
}
static GstPadProbeReturn
query_cb (GstPad * pad, GstPadProbeInfo * info, gpointer user_data)
{
    STREAM *stream = (STREAM*)user_data;
    GstQuery *query = GST_PAD_PROBE_INFO_QUERY (info);
    switch (GST_QUERY_TYPE (query)) {
        case GST_QUERY_CONTEXT:
        {
            if (gst_gl_handle_context_query (stream->pipeline, query,
                                             (GstGLDisplay *) stream->gst_display, NULL,
                                             (GstGLContext *) stream->gl_context)){
                return GST_PAD_PROBE_HANDLED;
            }
            break;
        }
        case GST_QUERY_DRAIN:
        {
            break;
        }
        default:
            break;
    }
    return GST_PAD_PROBE_OK;
}
static void
preroll_cb (GstElement * fakesink, GstBuffer * buffer, GstPad * pad, gpointer user_data)
{
    STREAM *stream = (STREAM*)user_data;
    queue_object (stream,GST_MINI_OBJECT_CAST (gst_buffer_ref (buffer)), FALSE);
}

static void
buffers_cb (GstElement * fakesink, GstBuffer * buffer, GstPad * pad, gpointer user_data)
{
    STREAM *stream = (STREAM*)user_data;
    queue_object (stream,GST_MINI_OBJECT_CAST (gst_buffer_ref (buffer)), SYNC_BUFFERS);
}

void create_pipeline(STREAM *stream){
    SpiceGstPlayFlags flags;
    GstPad *pad = NULL;
    GstPad *ghostpad = NULL;
    GstElement *vbin = gst_bin_new ("vbin");

    GstElement *glfilter = gst_element_factory_make ("glupload", "glfilter");
    GstElement *capsfilter = gst_element_factory_make ("capsfilter", NULL);
    GstElement *vsink = gst_element_factory_make ("fakesink", "vsink");

    g_object_set (capsfilter, "caps",
                  gst_caps_from_string ("video/x-raw(memory:GLMemory), format=RGBA"), NULL);

    g_object_set (vsink, "sync", FALSE, "silent", TRUE, "qos", FALSE,
                  "enable-last-sample", FALSE, "max-lateness", 200 * GST_MSECOND,
                  "signal-handoffs", TRUE, NULL);
    g_signal_connect (vsink, "preroll-handoff", G_CALLBACK (preroll_cb), stream);
    g_signal_connect (vsink, "handoff", G_CALLBACK (buffers_cb), stream);

    gst_bin_add_many (GST_BIN (vbin), glfilter, capsfilter, vsink, NULL);

    pad = gst_element_get_static_pad (glfilter, "sink");
    ghostpad = gst_ghost_pad_new ("sink", pad);
    gst_object_unref (pad);
    gst_element_add_pad (vbin, ghostpad);

    pad = gst_element_get_static_pad (vsink, "sink");
    gst_pad_add_probe (pad, GST_PAD_PROBE_TYPE_EVENT_DOWNSTREAM, events_cb, stream, NULL);
    gst_pad_add_probe (pad, GST_PAD_PROBE_TYPE_QUERY_DOWNSTREAM, query_cb,stream, NULL);
    gst_object_unref (pad);

    gst_element_link (glfilter, capsfilter);
    gst_element_link (capsfilter, vsink);

    stream->pipeline = gst_element_factory_make ("playbin", "player");
    g_object_set (stream->pipeline, "uri", stream->uri,
                  "video-sink", vbin,
                  NULL);
//    g_object_get(stream->pipeline, "flags", &flags, NULL);
//    flags &= ~(GST_PLAY_FLAG_AUDIO | GST_PLAY_FLAG_TEXT);
//    g_object_set(stream->pipeline, "flags", flags, NULL);
    stream->vsink = gst_object_ref (vsink);
    gst_element_set_state(stream->pipeline, GST_STATE_PLAYING);
}

gboolean handle_queued_objects (STREAM *stream)
{
    GstMiniObject *object = NULL;

    g_mutex_lock (&stream->queue_lock);
    if (stream->flushing) {
        g_cond_broadcast (&stream->cond);
        goto beach;
    } else if (g_async_queue_length (stream->queue) == 0) {
        goto beach;
    }
    if ((object = g_async_queue_try_pop (stream->queue))) {
        if (GST_IS_BUFFER (object)) {
            if (stream->current_buffer){
                gst_buffer_unref(stream->current_buffer);
                stream->current_buffer = NULL;
            }
            stream->current_buffer = GST_BUFFER_CAST (object);
            if (!SYNC_BUFFERS) {
                object = NULL;
            }
        } else if (GST_IS_EVENT (object)) {
            GstEvent *event = GST_EVENT_CAST (object);
            switch (GST_EVENT_TYPE (event)) {
                case GST_EVENT_EOS:
                    break;
                default:
                    break;
            }
            gst_event_unref (event);
            object = NULL;
        }
    }

    if (object) {
        stream->popped_obj = object;
        g_cond_broadcast (&stream->cond);
    }

    beach:
    g_mutex_unlock (&stream->queue_lock);

    return FALSE;
}