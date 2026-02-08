#include <gst/gst.h>
#include <stdlib.h>
#include <time.h>
#include <cstdio>


int main(int argc, char *argv[]) {

    printf("Buildling\n");
    GstElement *pipeline;
    GstBus *bus;
    GstMessage *msg;

    gst_init(&argc, &argv);
    srand(time(NULL));

    /* Define the GRAY8 pipeline string */
    printf("this is an update from 2022.1\n");
    const char* pipeline_cmd = 
        "v4l2src device=/dev/video0 io-mode=mmap ! h264parse ! video/x-h264,alignment=au ! "
        "queue ! omxh264dec low-latency=true ! "
        "video/x-raw, width=1920, height=1080, format=NV12 ! "
        "queue ! "
	    "vvas_xmultisrc kconfig=\"/opt/xilinx/kv260-smartcam/share/vvas/facedetect/nv122bgra.json\" ! "
	    "video/x-raw, width=1920, height=1080, format=RGBA ! "
        "queue ! "
        "vvas_xfilter name=text2overlay kernels-config=\"/opt/xilinx/kv260-smartcam/share/vvas/text2overlay.json\" ! "
        "queue ! "
        "kmssink bus-id=fd4a0000.display plane-id=40 sync=false can-scale=false";

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

    /* Play */
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, (GstMessageType)(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    /* Shutdown */
    if (msg != NULL) gst_message_unref(msg);
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
}