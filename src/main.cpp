#include <gst/gst.h>
#include <gst/vvas/gstinferencemeta.h>
#include <stdlib.h>
#include <time.h>
#include <cstdio>

/* These mimic your persistent hardware buffers (kernel_priv->mango_pix) */
static uint32_t g_mango = 0;
static uint32_t g_defect = 0;

/* Metadata Injection Probe: Replicates the xlnx_kernel_start logic */
static GstPadProbeReturn
inject_metadata_probe(GstPad *pad, GstPadProbeInfo *info, gpointer user_data) {
    GstBuffer *buffer = GST_PAD_PROBE_INFO_BUFFER(info);
    
    // Simulate new "FPGA" results
    g_mango = 50000 + (rand() % 5000);
    g_defect = rand() % 2000;

    /* --- START OF METADATA ALLOCATION (Similar to your example) --- */
    
    // 1. Add the meta container to the buffer
    GstInferenceMeta *infer_meta = (GstInferenceMeta *) gst_buffer_add_meta (
                                     buffer, 
                                     gst_inference_meta_get_info (), 
                                     NULL);

    // 2. Allocate root prediction
    infer_meta->prediction = gst_inference_prediction_new ();

    // 3. Allocate child prediction
    GstInferencePrediction *predict = gst_inference_prediction_new ();

    // 4. Assign pointers to the reserved fields (mimics your hardware pointers)
    predict->reserved_1 = (void *) &g_mango;
    predict->reserved_2 = (void *) &g_defect;

    // 5. Create and append classification (required for the overlay loop)
    GstInferenceClassification *cls = gst_inference_classification_new_full (
                                        -1, 0.0, "DEFECT DENSITY", 0, 
                                        NULL, NULL, NULL);
    gst_inference_prediction_append_classification (predict, cls);

    // 6. Append child to root (Ownership is transferred, recursive free is set)
    gst_inference_prediction_append (infer_meta->prediction, predict);

    /* --- END OF METADATA ALLOCATION --- */

    return GST_PAD_PROBE_OK;
}

int main(int argc, char *argv[]) {

    printf("Stuff got eliminated\n");
    GstElement *pipeline;
    GstBus *bus;
    GstMessage *msg;

    gst_init(&argc, &argv);
    srand(time(NULL));

    /* Define the GRAY8 pipeline string */
    const char* pipeline_cmd = 
        "v4l2src device=/dev/video0 io-mode=mmap ! h264parse ! video/x-h264,alignment=au ! "
        "queue ! omxh264dec low-latency=true ! "
        "video/x-raw, width=1280, height=720, format=NV12 ! "
        "queue name=source0 ! "
        "vvas_xfilter name=text2overlay kernels-config=\"/opt/xilinx/kv260-smartcam/share/vvas/text2overlay.json\" ! "
        "queue ! "
        "kmssink driver-name=xlnx plane-id=39 sync=false fullscreen-overlay=true";

    /* Print the pipeline for verification */
    printf("\n==================================================\n");
    printf("DEBUG: Launching GStreamer pipeline:\n%s\n", pipeline_cmd);
    printf("==================================================\n\n");

    /* Build the pipeline */
    pipeline = gst_parse_launch(pipeline_cmd, NULL);


        
    if (!pipeline) {
        g_printerr("Pipeline creation failed. Verify vvas_xfilter is in path.\n");
        return -1;
    }

    /* Get the pad and attach the metadata injector */
    GstElement *vsrc = gst_bin_get_by_name(GST_BIN(pipeline), "source0");
    GstPad *srcpad = gst_element_get_static_pad(vsrc, "src");
    gst_pad_add_probe(srcpad, GST_PAD_PROBE_TYPE_BUFFER, (GstPadProbeCallback)inject_metadata_probe, NULL, NULL);

    gst_object_unref(srcpad);
    gst_object_unref(vsrc);

    /* Play */
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    g_print("Simulation running. Text2Overlay is now consuming injected metadata.\n");

    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    /* Shutdown */
    if (msg != NULL) gst_message_unref(msg);
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}