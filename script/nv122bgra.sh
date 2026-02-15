# 1 = Manual Mode
v4l2-ctl -d /dev/video0 -c auto_exposure=1 
# Set a low value for fast shutter (Experiment with this number)
v4l2-ctl -d /dev/video0 -c exposure_time_absolute=100
#######################################################################################
SOURCE1=
SOURCE2=
#######################################################################################
DISPLAY1="kmssink plane-id=41"
DISPLAY2="kmssink driver-name=xlnx plane-id=39 sync=false fullscreen-overlay=true"
RUN=bgr7
FILE="multifilesink location=$RUN/capture%04d.bgr"
#######################################################################################
#R_1920x1080 5 | R_1280x720 10 | R_800x600 24 | R_640x480 30
mkdir -p $RUN
W=1920
H=1080
gst-launch-1.0 v4l2src device=/dev/video0 io-mode=mmap num-buffers=1000 ! \
	h264parse ! \
	"video/x-h264,alignment=au" ! \
	queue ! \
	omxh264dec low-latency=true ! \
	"video/x-raw, width=$W, height=$H, format=NV12" ! \
	queue ! \
	perf ! \
	vvas_xmultisrc kconfig="/opt/xilinx/kv260-smartcam/share/vvas/facedetect/nv122bgra.json" ! \
	video/x-raw, width=$W, height=$H, format=RGBA ! \
	queue ! \
    vvas_xfilter name=text2overlay kernels-config="/opt/xilinx/kv260-smartcam/share/vvas/text2overlay.json" ! \
    queue ! \
	$DISPLAY1 -v

#######################################################################################
#scp -r $KV:/home/petalinux/bgr6 ~/Videos/
