gst-launch-1.0 multifilesrc location=/home/wave/Videos/bgr7/capture%04d.bgr loop=true ! \
    "video/x-raw, format=BGR, width=1920, height=1080, framerate=30/1" ! \
    videoconvert ! \
    autovideosink