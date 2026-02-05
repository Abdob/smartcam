/*
 * Copyright 2021 Xilinx Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <glob.h>
#include <stdio.h>
#include <gst/gst.h>
#include <string>
#include <array>
#include <vector>
#include <sstream>
#include <memory>
#include <stdexcept>
#include <unistd.h>
#include <sys/types.h>

static gboolean my_bus_callback(GstBus *bus, GstMessage *message, gpointer data) {
    GMainLoop *loop = (GMainLoop *)data;
    switch (GST_MESSAGE_TYPE(message)) {
        case GST_MESSAGE_EOS:
            g_print("End of stream\n");
            g_main_loop_quit(loop);
            break;
        case GST_MESSAGE_ERROR: {
            GError *err;
            gchar *debug;
            gst_message_parse_error(message, &err, &debug);
            g_printerr("Error: %s\n", err->message);
            g_free(debug);
            g_error_free(err);
            g_main_loop_quit(loop);
            break;
        }
        default:
            break;
    }
    return TRUE;
}

int main(int argc, char *argv[]) {
    GMainLoop *loop;
    GstElement *pipeline;
    GstBus *bus;
    guint bus_watch_id;

    /* Initialize GStreamer */
    gst_init(&argc, &argv);
    loop = g_main_loop_new(NULL, FALSE);

    int width = 1920;
    int height = 1080;
    /* The simplified pipeline string */
    gchar *pip = g_strdup_printf(
        "v4l2src device=/dev/video0 io-mode=mmap ! h264parse ! video/x-h264,alignment=au ! "
        "queue ! omxh264dec low-latency=true ! video/x-raw, width=%d, height=%d ! "
        "tee name=t ! queue ! vvas_xmultisrc kconfig=\"/opt/xilinx/kv260-smartcam/share/vvas/facedetect/preprocess.json\" ! "
        "queue ! vvas_xfilter kernels-config=\"/opt/xilinx/kv260-smartcam/share/vvas/facedetect/aiinference.json\" ! "
        "ima.sink_master vvas_xmetaaffixer name=ima ima.src_master ! fakesink "
        "t. ! queue max-size-buffers=1 leaky=2 ! ima.sink_slave_0 ima.src_slave_0 ! "
        "queue ! vvas_xfilter kernels-config=\"/opt/xilinx/kv260-smartcam/share/vvas/facedetect/drawresult.json\" ! "
        "queue ! kmssink driver-name=xlnx plane-id=39 sync=false fullscreen-overlay=true", 
        width, height);

    /* Launch the pipeline */
    pipeline = gst_parse_launch(pip, NULL);
    if (!pipeline) {
        g_printerr("Failed to create pipeline.\n");
        return -1;
    }

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    /* Add a bus watch to handle messages */
    bus = gst_element_get_bus(pipeline);
    bus_watch_id = gst_bus_add_watch(bus, my_bus_callback, loop);
    gst_object_unref(bus);

    /* Run the loop */
    g_print("Running simplified pipeline: %s\n", pip);
    g_main_loop_run(loop);

    /* Clean up */
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_source_remove(bus_watch_id);
    g_main_loop_unref(loop);

    return 0;
}