#######################################################################################
SOURCE1=
SOURCE2=
#######################################################################################
DISPLAY1="kmssink plane-id=41"
DISPLAY2="kmssink driver-name=xlnx plane-id=39 sync=false fullscreen-overlay=true"
#######################################################################################
#R_1920x1080 5 | R_1280x720 10 | R_800x600 24 | R_640x480 30
W=1920
H=1080
gst-launch-1.0 -v filesrc location=mandelbrot_1080p.h264 ! \
	h264parse ! \
	"video/x-h264,alignment=au" ! \
	queue ! \
	omxh264dec low-latency=true ! \
	"video/x-raw, width=$W, height=$H, format=NV12" ! \
	queue ! \
	perf ! \
	vvas_xmultisrc kconfig="/opt/xilinx/kv260-smartcam/share/vvas/facedetect/nv122rgba.json" ! \
	video/x-raw, width=$W, height=$H, format=RGBA ! \
	queue ! \
	glpassthrough operation-mode=1 ! \
	queue ! \
	vvas_xmultisrc kconfig="/opt/xilinx/kv260-smartcam/share/vvas/facedetect/rgba2nv12.json" ! \
	video/x-raw, width=$W, height=$H, format=NV12 ! \
	$DISPLAY2

#######################################################################################
#scp -r $KV:/home/petalinux/bgr6 ~/Videos/
