gst-launch-1.0 -v videotestsrc ! \
    "video/x-raw, width=1920, height=1080, format=RGBA" ! \
    kmssink plane-id=41 \
    sync=false