gst-launch-1.0 multifilesrc location=/home/wave/Videos/bgr5/capture%04d.bgr loop=true ! \
    "video/x-raw, format=BGR, width=1280, height=720, framerate=30/1" ! \
    videoconvert ! \
    autovideosink