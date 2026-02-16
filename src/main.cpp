#include <gst/gst.h>
#include <stdlib.h>
#include <time.h>
#include <cstdio>

/* * Callback triggered when a specific metadata instance is about to be released.
 * This satisfies the "notify when metadata reached its end" requirement for 
 * each individual buffer.
 */
static void metadata_destroyed_notify(gpointer data) {
    GstStructure *s = (GstStructure *)data;
    const gchar* label = gst_structure_get_string(s, "label");
    int val;
    gst_structure_get_int(s, "value", &val);
    
    printf("[LIFECYCLE] RELEASING: %s (Value was: %d)\n", label, val);
    
    /* Safely free the structure now that the buffer is dead */
    gst_structure_free(s);
}

/* * Pad Probe: Generates extensible metadata at the queue's source pad.
 */
static GstPadProbeReturn
cb_generate_metadata (GstPad * pad, GstPadProbeInfo * info, gpointer user_data) {
    GstBuffer *buffer = GST_PAD_PROBE_INFO_BUFFER (info);
    if (!buffer) return GST_PAD_PROBE_OK;

    static int id_counter = 0;
    char id_name[64];
    sprintf(id_name, "TeraHertz_Meta_Batch_%d", id_counter++);

    /* 1. Create Extensible Data Structure */
    GstStructure *s = gst_structure_new("CustomData",
        "label", G_TYPE_STRING, id_name,
        "value", G_TYPE_INT, rand() % 100,
        "is_last_packet", G_TYPE_BOOLEAN, FALSE, // Could be set true on final frame
        NULL);

    /* 2. Attach pointer to GstReferenceTimestampMeta for the Kernel to read */
    gst_buffer_add_reference_timestamp_meta(buffer, 
        gst_caps_new_any(), 
        (GstClockTime)s, 
        GST_CLOCK_TIME_NONE);

    /* 3. Attach Lifecycle Notification.
    * We use a Quark to identify our data and gst_mini_object_set_qdata 
    * which is the correct way to handle lifecycle for GstBuffers.
    */
    static GQuark cleanup_quark = g_quark_from_static_string("cleanup_notify");

    gst_mini_object_set_qdata(
        GST_MINI_OBJECT(buffer), 
        cleanup_quark, 
        s, 
        (GDestroyNotify)metadata_destroyed_notify
    );

    printf("[LIFECYCLE] CREATED: %s\n", id_name);

    return GST_PAD_PROBE_OK;
}

int main(int argc, char *argv[]) {
    printf("Starting TeraHertz VVAS 3.0 Extensible Pipeline...\n");

    GstElement *pipeline, *queue_injector;
    GstBus *bus;
    GstMessage *msg;
    GstPad *srcpad;

    gst_init(&argc, &argv);
    srand(time(NULL));

    /* * Pipeline configuration:
     * We name the queue 'data_injector' so we can attach our probe to its source pad.
     */
    const char* pipeline_cmd = 
        "v4l2src device=/dev/video0 io-mode=mmap ! h264parse ! video/x-h264,alignment=au ! "
        "queue ! omxh264dec low-latency=true ! "
        "video/x-raw, width=1920, height=1080, format=NV12 ! "
        "queue ! "
        "vvas_xmultisrc kconfig=\"/opt/xilinx/kv260-smartcam/share/vvas/facedetect/nv122bgra.json\" ! "
        "video/x-raw, width=1920, height=1080, format=RGBA ! "
        "queue name=data_injector ! " 
        "vvas_xfilter name=text2overlay kernels-config=\"/opt/xilinx/kv260-smartcam/share/vvas/text2overlay.json\" ! "
        "queue ! "
        "kmssink plane-id=41";

    /* Build the pipeline */
    pipeline = gst_parse_launch(pipeline_cmd, NULL);
    if (!pipeline) {
        g_printerr("Failed to create pipeline. Check VVAS plugin installation.\n");
        return -1;
    }

    /* Locate the injector queue and attach the probe */
    queue_injector = gst_bin_get_by_name(GST_BIN(pipeline), "data_injector");
    srcpad = gst_element_get_static_pad(queue_injector, "src");
    
    gst_pad_add_probe(srcpad, GST_PAD_PROBE_TYPE_BUFFER, 
                     (GstPadProbeCallback)cb_generate_metadata, NULL, NULL);

    /* Cleanup pad references */
    gst_object_unref(srcpad);
    gst_object_unref(queue_injector);

    /* Start Playing */
    printf("DEBUG: Setting pipeline to PLAYING state...\n");
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    /* Wait for Error or End-of-Stream */
    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, 
                                    (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    /* Cleanup and Shutdown */
    if (msg != NULL) {
        if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
            GError *err;
            gchar *debug_info;
            gst_message_parse_error(msg, &err, &debug_info);
            g_printerr("Pipeline Error: %s\n", err->message);
            g_error_free(err);
            g_free(debug_info);
        }
        gst_message_unref(msg);
    }

    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    printf("TeraHertz Pipeline Shutdown Cleanly.\n");
    return 0;
}
