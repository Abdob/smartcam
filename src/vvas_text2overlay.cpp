uint32_t xlnx_kernel_start (VVASKernel * handle, int start,
      VVASFrame * input[MAX_NUM_OBJECT], VVASFrame * output[MAX_NUM_OBJECT])
  {
    vvas_xoverlaypriv *kpriv = (vvas_xoverlaypriv *) handle->kernel_priv;
    struct overlayframe_info *frameinfo = &(kpriv->frameinfo);
    frameinfo->inframe = input[0];

    // 1. Setup frame buffer
    char *lumaBuf = (char *) frameinfo->inframe->vaddr[0];
    frameinfo->lumaImg.create (input[0]->props.height, input[0]->props.stride/4, CV_8UC4);
    frameinfo->lumaImg.data = (unsigned char *) lumaBuf;

    // 2. Generate Rainbow Color based on frame count
    static uint32_t frame_count = 0;
    frame_count++;

    // Frequency controls how fast the colors change (lower = slower)
    float freq = 0.05; 
    double r = sin(freq * frame_count + 0) * 127 + 128;
    double g = sin(freq * frame_count + 2) * 127 + 128;
    double b = sin(freq * frame_count + 4) * 127 + 128;

    // 3. Metadata Extraction
    GstBuffer *buf = (GstBuffer *)frameinfo->inframe->app_priv;
    GstReferenceTimestampMeta *meta = gst_buffer_get_reference_timestamp_meta(buf, NULL);
    char text_buffer[512];

    if (meta) {
        GstStructure *s = (GstStructure *)meta->timestamp;
        const gchar* label = gst_structure_get_string(s, "label");
        sprintf(text_buffer, "CONSUME: %s", label ? label : "Unknown");
    } else {
        sprintf(text_buffer, "CONSUME: NO METADATA");
    }

    // 4. Draw with the dynamic Scalar color
    // OpenCV uses BGR order: Scalar(Blue, Green, Red)
    putText(frameinfo->lumaImg, text_buffer, 
            cv::Point(kpriv->x_offset, kpriv->y_offset), 
            kpriv->font, kpriv->font_size, 
            Scalar(b, g, r), 2, LINE_AA); // Increased thickness/quality for better visibility

    return 0;
  }