#####################################################################################
DISPLAY1="kmssink plane-id=41 sync=false can-scale=false"
DISPLAY2="kmssink driver-name=xlnx plane-id=39 sync=false fullscreen-overlay=true"
#####################################################################################
#R_1920x1080 5 | R_1280x720 10 | R_800x600 24 | R_640x480 30
W=1920
H=1080
gst-launch-1.0 v4l2src device=/dev/video2 io-mode=mmap num-buffers=1000 ! \
	h264parse ! \
	"video/x-h264,alignment=au" ! \
	queue ! \
	omxh264dec low-latency=true ! \
	"video/x-raw, width=$W, height=$H" ! \
	vvas_xmultisrc kconfig="/opt/xilinx/kv260-smartcam/share/vvas/facedetect/nv122rgba.json" ! \
    video/x-raw, width=$W, height=$H, format=RGBA ! \
	queue ! \
	glpassthrough operation-mode=1 ! \
	$DISPLAY1

#######################################################################################