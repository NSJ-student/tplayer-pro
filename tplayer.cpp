#include "tplayer.h"

TPlayer::TPlayer(WId winid)
{
    GstCaps *filter;

    /* Initialize GStreamer */
    gst_init(NULL, NULL);
    memset(&play_obj, 0, sizeof(play_obj));
    // Set output media

    play_obj.state = STATE_PREPARING;
    /***********************************/
    //
    // Gstreamer Pipe
    //
    // [source] -> [typefind] -> [queue] -->
    // --(image)--> [decode] ------------->
    // --(video)--> [demux]  -> [decode] -> [textoverlay] -> [capsfilter] -> [sink]
    //
    /***********************************/

    // Make source element
    play_obj.source = gst_element_factory_make ("filesrc", "file-source");
    play_obj.typefind = gst_element_factory_make ("typefind", "src-typefind");
    play_obj.queue = gst_element_factory_make ("queue", "src-queue");

    g_signal_connect (play_obj.typefind, "have-type", G_CALLBACK(st_type_find_handler), this);

    // Make sink element
    play_obj.textoverlay = gst_element_factory_make ("textoverlay", "default-text");
    play_obj.caps = gst_element_factory_make ("capsfilter", NULL);
    play_obj.sink = gst_element_factory_make ("d3dvideosink", "test-sink");

    g_object_set(G_OBJECT(play_obj.textoverlay),
            "halignment", 0, "valignment", 2,
            "deltax", 0, "deltay", 0,
            "font-desc", "Sans, 10",
            NULL);
    filter = gst_caps_from_string("video/x-raw");//,width=3840,height=2160");
    g_object_set(G_OBJECT(play_obj.caps), "caps", filter, NULL);
    if(winid != NULL)
    {
        gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(play_obj.sink), winid);
    }
/*
    g_object_set(G_OBJECT(play_obj.sink),
            "force-aspect-ratio", TRUE,
            "display", ":0",
            "fullscreen", FALSE,
            //"sync", FALSE,
            //"async", FALSE,
            //"show-preroll-frame", FALSE,
            //"double-buffer", TRUE,
            "window-pos-x", 0,
            "window-pos-y", 0,
            "console-debug", TRUE,
            "change-size", TRUE,
            "handle-expose", TRUE,
            "handle-events", TRUE, NULL);
*/
    // Make Pipeline
    play_obj.pipeline = gst_pipeline_new ("test-pipeline");
    if (!play_obj.pipeline || !play_obj.sink || !play_obj.source
            || !play_obj.caps || !play_obj.typefind)
    {
        debug_msg ("Not all elements could be created.\n");
        return;
    }

    // Build the pipeline
    gst_bin_add_many (GST_BIN (play_obj.pipeline),
            play_obj.source, play_obj.typefind, play_obj.queue,
            play_obj.textoverlay, play_obj.caps, play_obj.sink, NULL);

    gst_element_link_many (play_obj.source, play_obj.typefind, play_obj.queue, NULL);
    gst_element_link_many (play_obj.textoverlay, play_obj.caps, play_obj.sink, NULL);
#if (USE_POP_MODE == 0)
    make_all_decode_bin(&play_obj, st_pad_added_handler, st_pad_removed_handler);
#endif

    // Wait until error or EOS
    play_obj.bus = gst_pipeline_get_bus (GST_PIPELINE(play_obj.pipeline));
    gst_bus_set_sync_handler(play_obj.bus, (GstBusSyncHandler)st_my_bus_callback, &play_obj, NULL);

    play_obj.state = STATE_STOP;
    // Start playing
/*
    play_obj.loop = g_main_loop_new(NULL, FALSE);
    play_type=PLAY_START;
    g_idle_add((GSourceFunc)play_start, &play_obj);
    g_main_loop_run(play_obj.loop);
*/
    return;
}

TPlayer::~TPlayer()
{
    // Free resources
    debug_msg("+++++ Stop Play\n");
    gst_object_unref (play_obj.bus);
    gst_element_set_state (play_obj.pipeline, GST_STATE_NULL);
    gst_object_unref (play_obj.pipeline);
/*
    for(; img_cnt > 0; img_cnt--)
        free(image_list[img_cnt-1]);
    for(; video_cnt > 0; video_cnt--)
        free(video_list[video_cnt-1]);
        */
}

GstBusSyncReply TPlayer::my_bus_callback(GstBus *bus, GstMessage *message, void * arg)
{
    PipeElement * obj = (PipeElement *)arg;
    GError * err = NULL;
    GstState oldstate, newstate;

    debug_msg("my_bus_callback");
    debug_msg("0x%X  <%s>\n", GST_MESSAGE_TYPE(message), GST_OBJECT_NAME(message->src));
    switch(GST_MESSAGE_TYPE(message))
    {
        case GST_MESSAGE_ERROR:
            debug_msg("GST_MESSAGE_ERROR => ");
            gst_message_parse_error(message, &err, NULL);
            debug_msg("%s: %s \n", GST_OBJECT_NAME(message->src), err->message);
            play_stop(obj);
            break;
        case GST_MESSAGE_EOS:
            debug_msg("GST_MESSAGE_EOS  <%s>\n", GST_OBJECT_NAME(message->src));
            play_next(obj);
            break;
        case GST_MESSAGE_STATE_CHANGED:
            gst_message_parse_state_changed(message, &oldstate, &newstate, NULL);
            if(g_str_has_prefix(GST_OBJECT_NAME(message->src), "test-sink"))
            {
                if(newstate == GST_STATE_PLAYING)
                    obj->state = STATE_PLAYING;
                else if(newstate == GST_STATE_READY)
                    obj->state = STATE_PAUSE;
                else
                    obj->state = STATE_STOP;
            }
            break;
    }

    return GST_BUS_PASS;
}

/* This function will be called by the pad-added signal */
void TPlayer::pad_added_handler (GstElement *src, GstPad *new_pad, void *data)
{
    PipeElement * obj = &(((TPlayer *)data)->play_obj);
    GstCaps *new_pad_caps = NULL;
    GstStructure *new_pad_struct = NULL;
    const gchar *new_pad_type = NULL;
    int ret;

    debug_msg("+++++ Demux Pad-Added: %s\n", GST_OBJECT_NAME(src));
    /* Check the new pad's type */
    new_pad_caps = gst_pad_get_current_caps (new_pad);
    new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
    new_pad_type = gst_structure_get_name (new_pad_struct);

    if (g_str_has_prefix (new_pad_type, "video/"))
    {
        // Make decode bin
        ret = link_video_decode(obj, new_pad_type, new_pad);
        if (ret == LINK_SUCCESS)
        {
        }
        else
        {
        }
    }
    else
    {
        g_print ("  It has type '%s' which is not raw audio. Ignoring.\n", new_pad_type);
        goto exit;
    }

exit:
    /* Unreference the new pad's caps, if we got them */
    if (new_pad_caps != NULL)
        gst_caps_unref (new_pad_caps);
}

void TPlayer::pad_removed_handler (GstElement *src, GstPad *new_pad, void *data)
{
    gchar *element_name = gst_element_get_name(src);

    debug_msg("Pad <%s> Removed...\n", element_name);
    //free(element_name);
}

void TPlayer::type_find_handler (GstElement *typefind, guint probability, GstCaps *caps, void *data)
{
    PipeElement * obj = &(((TPlayer *)data)->play_obj);
    GstStateChangeReturn ret;
    char * type;

    type = gst_caps_to_string(caps);

    debug_msg("** TypeFind [%s]\n", type);
    if(g_str_has_prefix(type, "image"))
    {
        if(link_image_decode(obj, type) == LINK_SUCCESS)
            ret = gst_element_set_state (obj->pipeline, GST_STATE_PLAYING);
        else
        {
            debug_msg("typefind: link_image_decode failed\n");
            return;
           // exit(0);
        }
    }
    else if(g_str_has_prefix(type, "video"))
    {
        if(link_video_demux(obj, type) == LINK_FAIL)
        {
            ret = GST_STATE_CHANGE_FAILURE;
            debug_msg("typefind: link_video_demux failed\n");
            return;
           // exit(0);
        }
    }
    else
    {
        debug_msg("  No Available Type: [ %s ]\n", type);
    }

    g_free(type);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr ("  Unable to set the pipeline to the playing state.\n");
        gst_object_unref (obj->pipeline);
        return;
    }
}

#if (USE_POP_MODE == 1)

int TPlayer::select_decode_bin(GstElement ** bin, char * type)
{
    GstElement * selected;

    if	(g_str_has_prefix(type, "image/jpeg"))		make_jpg_bin(&selected);
    else if	(g_str_has_prefix(type, "image/bmp"))		make_bmp_bin(&selected);
    else if	(g_str_has_prefix(type, "video/x-h264"))	make_h264_bin(&selected);
    else if (g_str_has_prefix(type, "video/mpeg"))		make_mpeg_bin(&selected);
    else		selected = NULL;

    *bin = selected;
    return TRUE;

}
int TPlayer::select_demux_bin(GstElement ** bin, char * type)
{
    GstElement * selected;

    if(g_str_has_prefix(type, "video/quicktime"))
        selected = gst_element_factory_make ("qtdemux", NULL);
    else if(g_str_has_prefix(type, "video/x-matroska"))
        selected = gst_element_factory_make ("matroskademux", NULL);
    else if(g_str_has_prefix(type, "video/mpegts"))
        selected = gst_element_factory_make ("tsdemux", NULL);
    else if(g_str_has_prefix(type, "video/mpeg"))
        selected = gst_element_factory_make ("mpegpsdemux", NULL);
    else		selected = NULL;

    *bin = selected;
    return TRUE;
}

#else	/* USE_POP_MODE */

void TPlayer::make_all_decode_bin(gpointer user_data, void *add_handler, void *remove_handler)
{
    PipeElement * obj = (PipeElement *)user_data;

    make_jpg_bin(&jpg_bin);
    make_bmp_bin(&bmp_bin);
    make_h264_bin(&h264_bin);
    make_mpeg_bin(&mpeg_bin);
    qtdemux = gst_element_factory_make ("qtdemux", NULL);
    matroskademux = gst_element_factory_make ("matroskademux", NULL);
    tsdemux = gst_element_factory_make ("tsdemux", NULL);
    mpegpsdemux = gst_element_factory_make ("mpegpsdemux", NULL);
    g_signal_connect (qtdemux, "pad-added", G_CALLBACK (add_handler), this);
    g_signal_connect (qtdemux, "pad-removed", G_CALLBACK (remove_handler), this);
    g_signal_connect (matroskademux, "pad-added", G_CALLBACK (add_handler), this);
    g_signal_connect (matroskademux, "pad-removed", G_CALLBACK (remove_handler), this);
    g_signal_connect (tsdemux, "pad-added", G_CALLBACK (add_handler), this);
    g_signal_connect (tsdemux, "pad-removed", G_CALLBACK (remove_handler), this);
    g_signal_connect (mpegpsdemux, "pad-added", G_CALLBACK (add_handler), this);
    g_signal_connect (mpegpsdemux, "pad-removed", G_CALLBACK (remove_handler), this);

    gst_bin_add_many (GST_BIN (obj->pipeline),
        jpg_bin, bmp_bin, h264_bin, mpeg_bin,
        qtdemux, matroskademux, tsdemux, mpegpsdemux, NULL);
    all_bin_created = TRUE;
}

int TPlayer::select_decode_bin(GstElement ** bin, char * type)
{
    GstElement * selected;
    // if decode bin is not created
    if(all_bin_created == FALSE)
    {
        *bin = NULL;
        return FALSE;
    }

    if	(g_str_has_prefix(type, "image/jpeg"))		selected = jpg_bin;
    else if	(g_str_has_prefix(type, "image/bmp"))		selected = bmp_bin;
    else if	(g_str_has_prefix(type, "video/x-h264"))	selected = h264_bin;
    else if (g_str_has_prefix(type, "video/mpeg"))		selected = mpeg_bin;
    else		selected = NULL;

    *bin = selected;
    return TRUE;
}
int TPlayer::select_demux_bin(GstElement ** bin, char * type)
{
    GstElement * selected;
    // if decode bin is not created
    if(all_bin_created == FALSE)
    {
        *bin = NULL;
        return FALSE;
    }

    if(g_str_has_prefix(type, "video/quicktime"))		selected = qtdemux;
    else if(g_str_has_prefix(type, "video/x-matroska"))	selected = matroskademux;
    else if(g_str_has_prefix(type, "video/mpegts"))		selected = tsdemux;
    else if(g_str_has_prefix(type, "video/mpeg"))		selected = mpegpsdemux;
    else		selected = NULL;

    *bin = selected;
    return TRUE;
}

#endif	/* USE_POP_MODE */


int TPlayer::make_jpg_bin(GstElement ** bin)
{
    GstElement *jpg_dec, *jpg_freeze, *jpg_convert, *jpg_scale;
    GstPad *tmp_pad;

    // Make jpg_bin
    *bin = gst_pipeline_new ("jpg_bin");
    CHECK_ELEMENT_CREATED(*bin, "jpg_bin", failed);
    // Make elements
    jpg_dec = gst_element_factory_make 	("jpegdec", NULL);
    CHECK_ELEMENT_CREATED(jpg_dec, "jpegdec", failed);

    jpg_convert = gst_element_factory_make 	("videoconvert", NULL);
    CHECK_ELEMENT_CREATED(jpg_convert, "videoconvert", failed);

    jpg_scale = gst_element_factory_make 	("videoscale", NULL);
    CHECK_ELEMENT_CREATED(jpg_scale, "videoscale", failed);

    jpg_freeze = gst_element_factory_make 	("imagefreeze", NULL);
    CHECK_ELEMENT_CREATED(jpg_freeze, "imagefreeze", failed);

    g_object_set(G_OBJECT(jpg_scale), "method", 3, NULL);
    // Add elements to bin
    gst_bin_add_many (GST_BIN (*bin), jpg_dec, jpg_convert, jpg_scale, jpg_freeze, NULL);
    gst_element_link_many (jpg_dec, jpg_convert, jpg_scale, jpg_freeze, NULL);

    tmp_pad = gst_element_get_static_pad(jpg_dec, "sink");
    gst_element_add_pad(*bin, gst_ghost_pad_new("sink", tmp_pad));
    gst_object_unref(GST_OBJECT(tmp_pad));

    tmp_pad = gst_element_get_static_pad(jpg_freeze, "src");
    gst_element_add_pad(*bin, gst_ghost_pad_new("src", tmp_pad));
    gst_object_unref(GST_OBJECT(tmp_pad));

    return TRUE;

failed:
    *bin = NULL;
    return FALSE;
}

int TPlayer::make_bmp_bin(GstElement ** bin)
{
    GstElement *bmp_dec, *bmp_freeze, *bmp_convert, *bmp_scale;
    GstPad *tmp_pad;

    // Make jpg_bin
    *bin = gst_pipeline_new ("bmp_bin");
    CHECK_ELEMENT_CREATED(*bin, "bmp_bin", failed);
    // Make elements
    bmp_dec = gst_element_factory_make 	("gdkpixbufdec", NULL);
    CHECK_ELEMENT_CREATED(bmp_dec, "gdkpixbufdec", failed);

    bmp_convert = gst_element_factory_make 	("autovideoconvert", NULL);
    CHECK_ELEMENT_CREATED(bmp_convert, "autovideoconvert", failed);

    bmp_scale = gst_element_factory_make 	("videoscale", NULL);
    CHECK_ELEMENT_CREATED(bmp_scale, "videoscale", failed);

    bmp_freeze = gst_element_factory_make 	("imagefreeze", NULL);
    CHECK_ELEMENT_CREATED(bmp_freeze, "imagefreeze", failed);

    g_object_set(G_OBJECT(bmp_scale), "method", 3, NULL);
    // Add elements to bin
    gst_bin_add_many (GST_BIN (*bin), bmp_dec, bmp_convert, bmp_scale, bmp_freeze, NULL);
    gst_element_link_many (bmp_dec, bmp_convert, bmp_scale, bmp_freeze, NULL);

    tmp_pad = gst_element_get_static_pad(bmp_dec, "sink");
    gst_element_add_pad(*bin, gst_ghost_pad_new("sink", tmp_pad));
    gst_object_unref(GST_OBJECT(tmp_pad));

    tmp_pad = gst_element_get_static_pad(bmp_freeze, "src");
    gst_element_add_pad(*bin, gst_ghost_pad_new("src", tmp_pad));
    gst_object_unref(GST_OBJECT(tmp_pad));

    return TRUE;

failed:
    *bin = NULL;
    return FALSE;
}

int TPlayer::make_h264_bin(GstElement ** bin)
{
    GstElement *h264_queue, *h264_queue2, *h264_queue3;
    GstElement *h264_h264parse, *h264_omxh264dec, *h264_nvconv;
    GstPad *tmp_pad;

    // Make h264_bin
    *bin = gst_pipeline_new ("h264_bin");
    CHECK_ELEMENT_CREATED(*bin, "h264_bin", failed);
    // Make elements
    //h264_nvconv = gst_element_factory_make ("nvvidconv", NULL);
    h264_nvconv = gst_element_factory_make ("videoconvert", NULL);
    CHECK_ELEMENT_CREATED(h264_nvconv, "h264_nvconv", failed);

    //h264_omxh264dec = gst_element_factory_make ("omxh264dec", NULL);
    //h264_omxh264dec = gst_element_factory_make ("openh264dec", NULL);
    h264_omxh264dec = gst_element_factory_make ("avdec_h264", NULL);
    CHECK_ELEMENT_CREATED(h264_omxh264dec, "openh264dec", failed);

    //h264_omxh264dec = gst_element_factory_make ("omxmpeg4videodec", NULL);
    h264_h264parse = gst_element_factory_make ("h264parse", NULL);
    CHECK_ELEMENT_CREATED(h264_h264parse, "h264_h264parse", failed);

    //h264_h264parse = gst_element_factory_make ("mpeg4videoparse", NULL);
    h264_queue = gst_element_factory_make ("queue", "h264_queue1");
    CHECK_ELEMENT_CREATED(h264_queue, "h264_queue", failed);

    h264_queue2 = gst_element_factory_make ("queue", "h264_queue2");
    CHECK_ELEMENT_CREATED(h264_queue2, "h264_queue2", failed);

    h264_queue3 = gst_element_factory_make ("queue", "h264_queue3");
    CHECK_ELEMENT_CREATED(h264_queue3, "h264_queue3", failed);

    //g_object_set(G_OBJECT(h264_queue), "flush-on-eos", TRUE, NULL);
    g_object_set(G_OBJECT(h264_queue2), "flush-on-eos", TRUE, NULL);
    g_object_set(G_OBJECT(h264_queue3), "flush-on-eos", TRUE, NULL);

    // Add elements to bin
    gst_bin_add_many (GST_BIN (*bin),
        h264_queue,
        h264_omxh264dec,
        h264_queue2,
        h264_h264parse,
        h264_queue3,
        h264_nvconv,
        NULL);
    gst_element_link_many (
        h264_queue,
        h264_h264parse,
        h264_queue2,
        h264_omxh264dec,
        h264_queue3,
        h264_nvconv,
        NULL);

    //tmp_pad = gst_element_get_static_pad(h264_queue, "sink");
    tmp_pad = gst_element_get_static_pad(h264_queue, "sink");
    gst_element_add_pad(*bin, gst_ghost_pad_new("sink", tmp_pad));
    gst_object_unref(GST_OBJECT(tmp_pad));

    tmp_pad = gst_element_get_static_pad(h264_nvconv, "src");
    gst_element_add_pad(*bin, gst_ghost_pad_new("src", tmp_pad));
    gst_object_unref(GST_OBJECT(tmp_pad));

    return TRUE;
failed:
    *bin = NULL;
    return FALSE;
}

int TPlayer::make_mpeg_bin(GstElement ** bin)
{
    GstElement *mpeg_queue, *mpeg_queue2, *mpeg_queue3;
    GstElement *mpeg_mpegvideoparse, *mpeg_omxmpeg2videodec, *mpeg_nvconv;
    GstPad *tmp_pad;

    // Make mpeg_bin
    *bin = gst_pipeline_new ("mpeg_bin");
    CHECK_ELEMENT_CREATED(*bin, "mpeg_bin", failed);
    // Make elements
    mpeg_mpegvideoparse = gst_element_factory_make ("mpegvideoparse", NULL);
    CHECK_ELEMENT_CREATED(mpeg_mpegvideoparse, "mpeg_mpegvideoparse", failed);

    //mpeg_omxmpeg2videodec = gst_element_factory_make ("omxmpeg2videodec", NULL);
    mpeg_omxmpeg2videodec = gst_element_factory_make ("mpeg2dec", NULL);
    CHECK_ELEMENT_CREATED(mpeg_omxmpeg2videodec, "mpeg2dec", failed);

    //mpeg_nvconv = gst_element_factory_make ("nvvidconv", NULL);
    mpeg_nvconv = gst_element_factory_make ("videoconvert", NULL);
    CHECK_ELEMENT_CREATED(mpeg_nvconv, "mpeg_videoconvert", failed);

    mpeg_queue = gst_element_factory_make ("queue", "mpeg_queue1");
    CHECK_ELEMENT_CREATED(mpeg_queue, "mpeg_queue", failed);

    mpeg_queue2 = gst_element_factory_make ("queue", "mpeg_queue2");
    CHECK_ELEMENT_CREATED(mpeg_queue2, "mpeg_queue", failed);

    mpeg_queue3 = gst_element_factory_make ("queue", "mpeg_queue3");
    CHECK_ELEMENT_CREATED(mpeg_queue3, "mpeg_queue", failed);

    g_object_set(G_OBJECT(mpeg_queue), "flush-on-eos", TRUE, NULL);
    g_object_set(G_OBJECT(mpeg_queue2), "flush-on-eos", TRUE, NULL);
    g_object_set(G_OBJECT(mpeg_queue3), "flush-on-eos", TRUE, NULL);
    // Add elements to bin
    gst_bin_add_many (GST_BIN (*bin),
        mpeg_queue,
        mpeg_mpegvideoparse,
        mpeg_queue2,
        mpeg_omxmpeg2videodec,
        mpeg_queue3,
        mpeg_nvconv,
        NULL);
    gst_element_link_many (
        mpeg_queue,
        mpeg_mpegvideoparse,
        mpeg_queue2,
        mpeg_omxmpeg2videodec,
        mpeg_queue3,
        mpeg_nvconv,
        NULL);

    tmp_pad = gst_element_get_static_pad(mpeg_queue, "sink");
    //tmp_pad = gst_element_get_static_pad(mpeg_mpegvideoparse, "sink");
    gst_element_add_pad(*bin, gst_ghost_pad_new("sink", tmp_pad));
    gst_object_unref(GST_OBJECT(tmp_pad));

    tmp_pad = gst_element_get_static_pad(mpeg_nvconv, "src");
    gst_element_add_pad(*bin, gst_ghost_pad_new("src", tmp_pad));
    gst_object_unref(GST_OBJECT(tmp_pad));

    if(//!mpeg_queue || !mpeg_queue2 || !mpeg_queue3 ||
        !mpeg_omxmpeg2videodec || !mpeg_mpegvideoparse)
    {
        debug_msg("Error: mpeg_bin \n");
        *bin = NULL;
        return FALSE;
    }

    return TRUE;
failed:
    *bin = NULL;
    return FALSE;
}


gboolean TPlayer::play_start(gpointer user_data)
{
    PipeElement * obj = (PipeElement *)user_data;
    char * play_path;

    fp_get_play_path(&play_path);
    if(play_path == NULL)
    {
        return FALSE;
    }

    if(obj->state != STATE_PLAYING)
    {
        obj->state = STATE_PREPARING;
        gst_element_set_state (obj->pipeline, GST_STATE_READY);

        g_object_set(G_OBJECT(obj->source),
                "location", play_path, NULL);

        g_object_set(G_OBJECT(obj->textoverlay),
                "text", play_path, NULL);

        debug_msg("\n+++++ Start Play %s\n", play_path);
        gst_element_set_state (obj->source, GST_STATE_PLAYING);
        gst_element_set_state (obj->typefind, GST_STATE_PLAYING);
    }
    else
        qDebug("can't play");

    free(play_path);
    return FALSE;
}

gboolean TPlayer::play_stop(gpointer user_data)
{
    PipeElement * obj = (PipeElement *)user_data;

    if((obj->state == STATE_PLAYING)||(obj->state == STATE_PAUSE))
    {
        obj->state = STATE_STOP;
        gst_element_set_state (obj->pipeline, GST_STATE_READY);
        obj->end_flag = TRUE;
    }

    return FALSE;
}

gboolean TPlayer::play_resume(gpointer user_data)
{
    PipeElement * obj = (PipeElement *)user_data;

    if(obj->state == STATE_PAUSE)
    {
        gst_element_set_state (obj->pipeline, GST_STATE_PLAYING);
    }

    return FALSE;
}

gboolean TPlayer::play_restart(gpointer user_data)
{
    PipeElement * obj = (PipeElement *)user_data;

    if((obj->state == STATE_PLAYING)||(obj->state == STATE_PAUSE))
    {
        obj->state = STATE_STOP;
    }

    return FALSE;
}

gboolean TPlayer::play_next(gpointer user_data)
{
    PipeElement * obj = (PipeElement *)user_data;

    if(obj->state == STATE_PLAYING)
    {
        /*
        if(*obj->play_cnt-1 > obj->play_idx)	obj->play_idx++;
        else					obj->play_idx = 0;
        */
        fp_play_next();
#if (USE_POP_MODE == 0)
        qDebug("to Next");
        gst_element_set_state (obj->pipeline, GST_STATE_READY);
        play_start(obj);
#else
        obj->state = STATE_STOP;
#endif
    }

    return FALSE;
}

gboolean TPlayer::play_prev(gpointer user_data)
{
    PipeElement * obj = (PipeElement *)user_data;

    if(obj->state == STATE_PLAYING)
    {
        obj->state = STATE_STOP;
        /*
        if(0 < obj->play_idx)	obj->play_idx--;
        else					obj->play_idx = *obj->play_cnt - 1;
        */
        fp_play_prev();
#if (USE_POP_MODE == 0)
        gst_element_set_state (obj->pipeline, GST_STATE_READY);
        play_start(obj);
#else
        obj->state = STATE_STOP;
#endif
    }

    return FALSE;
}

void TPlayer::set_callback(f_play_others prev, f_play_others next, f_get_play_object get_path)
{
    fp_play_next = std::forward<f_play_others>(next);
    fp_play_prev = std::forward<f_play_others>(prev);
    fp_get_play_path = std::forward<f_get_play_object>(get_path);
}

int TPlayer::link_image_decode(gpointer user_data, char * type)
{
    PipeElement * obj = (PipeElement *)user_data;
    GstElement * element = NULL;

    if(FALSE == select_decode_bin(&(element), type))
        return LINK_FAIL;

    if(element == NULL)
    {
        debug_msg("  No Available Decoder: [ %s ]\n", type);
        return LINK_FAIL;
    }
#if (USE_POP_MODE == 0)
    if(obj->decode != NULL)
    {
        gst_element_unlink (obj->queue, obj->demux);
        gst_element_unlink (obj->queue, obj->decode);
        gst_element_unlink (obj->decode, obj->textoverlay);
    }
    obj->decode = element;
#else
    gst_bin_add(GST_BIN(obj->pipeline), element);
#endif

    if(FALSE == gst_element_link (obj->queue, element))
    {
        debug_msg("  Image[%s] Link Failed (queue)...\n", type);
        gst_object_unref (element);
        return LINK_FAIL;
    }
    if(FALSE == gst_element_link (element, obj->textoverlay))
    {
        debug_msg("  Image[%s] Link Failed (textoverlay)...\n", type);
        gst_object_unref (element);
        return LINK_FAIL;
    }

    return LINK_SUCCESS;
}

int TPlayer::link_video_decode(gpointer user_data, const char * type, GstPad *new_pad)
{
    PipeElement * obj = (PipeElement *)user_data;
    GstElement * element = NULL;
    GstPad *sink_pad_video = NULL;
    GstPadLinkReturn ret;

    // Select decode bin
    if(FALSE == select_decode_bin(&(element), (char *)type))
        return LINK_FAIL;

    if(element == NULL)
    {
        debug_msg("  No Available Decoder: [ %s ]\n", type);
        return LINK_FAIL;
    }
#if (USE_POP_MODE == 0)
    if(obj->decode != NULL)
        gst_element_unlink (obj->decode, obj->textoverlay);
    obj->decode = element;
#else
    gst_bin_add(GST_BIN(obj->pipeline), element);
#endif

    // Gst decode pad
    sink_pad_video = gst_element_get_static_pad (element, "sink");
    if(sink_pad_video == NULL)
    {
        debug_msg("  Fail to get static pad... [%s]\n", type);
        return LINK_FAIL;
    }
    if(gst_pad_is_linked(sink_pad_video))
    {
        debug_msg("  Already Started... [%s]\n", type);
        /* Unreference the sink pad */
        gst_object_unref (sink_pad_video);
        return LINK_FAIL;
    }
    // Add & Link decode pad to play_pipeline
    ret = gst_pad_link (new_pad, sink_pad_video);
    if (GST_PAD_LINK_FAILED (ret))
    {
        debug_msg("  Type is '%s' but link failed.\n", type);
        gst_object_unref (sink_pad_video);
        return LINK_FAIL;
    }
    else
    {
        debug_msg("  Link succeeded (type '%s').\n", type);
        debug_msg("  ==> Play Start\n");
        if(FALSE == gst_element_link (element, obj->textoverlay))
        {
            debug_msg("  Video[%s] Link Failed (textoverlay)...\n", type);
            gst_object_unref (sink_pad_video);
            return LINK_FAIL;
        }
        gst_element_set_state (obj->pipeline, GST_STATE_PLAYING);
        obj->timer_flag = TRUE;

        gst_object_unref (sink_pad_video);
        return LINK_SUCCESS;
    }
}

int TPlayer::link_video_demux(gpointer user_data, char * type)
{
    PipeElement * obj = (PipeElement *)user_data;
    GstElement * element = NULL;

    // Select demux bin
    if(FALSE == select_demux_bin(&(element), type))
        return LINK_FAIL;

    if(element == NULL)
    {
        debug_msg("  No Available Demux: [ %s ]\n", type);
        return LINK_FAIL;
    }
#if (USE_POP_MODE == 0)
    if(obj->demux != NULL)
        gst_element_unlink (obj->queue, obj->demux);
    if(obj->decode != NULL)
        gst_element_unlink (obj->queue, obj->decode);

    obj->demux = element;
#else
    gst_bin_add(GST_BIN(obj->pipeline), element);
#endif
    if(FALSE == gst_element_link (obj->queue, element))
    {
        debug_msg("  Video Link Failed: [ %s ]\n", type);
        gst_object_unref (element);
        return LINK_FAIL;
    }

    if (gst_element_set_state (obj->queue, GST_STATE_PLAYING) ==
                GST_STATE_CHANGE_FAILURE)
    {
        debug_msg ("  Unable to set the pipeline to the playing state.\n");
        gst_object_unref (element);
        return LINK_FAIL;
    }
    if (gst_element_set_state (element, GST_STATE_PLAYING) ==
                GST_STATE_CHANGE_FAILURE)
    {
        debug_msg ("  Unable to set the pipeline to the playing state.\n");
        gst_object_unref (element);
        return LINK_FAIL;
    }

    return LINK_SUCCESS;
}
