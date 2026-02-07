gst-launch-1.0 multifilesrc location=/home/wave/Videos/bgr3/capture%04d.bgr loop=true ! \
    "video/x-raw, format=BGR, width=640, height=480, framerate=30/1" ! \
    videoconvert ! \
    autovideosink