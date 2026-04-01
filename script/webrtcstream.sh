IP_ADDR=192.168.10.198
CAM_DEVICE=2
SUB_X=100
SUB_Y=100
SIDE_X=100
SIDE_Y=100

#####################################################################################
DISPLAY1="kmssink plane-id=41 sync=false can-scale=false"
DISPLAY2="kmssink driver-name=xlnx plane-id=39 sync=false fullscreen-overlay=true"
#####################################################################################
#R_1920x1080 5 | R_1280x720 10 | R_800x600 24 | R_640x480 30
W=1920
H=1080
gst-launch-1.0 v4l2src device=/dev/video$CAM_DEVICE io-mode=mmap ! \
	h264parse ! \
	"video/x-h264,alignment=au" ! \
	omxh264dec low-latency=true ! \
	"video/x-raw, width=$W, height=$H" ! \
	vvas_xmultisrc kconfig="/opt/xilinx/kv260-smartcam/share/vvas/facedetect/nv122rgba.json" ! \
	video/x-raw, width=$W, height=$H, format=RGBA ! \
	thzmeta ! \
	timeoverlay ! \
	glpassthrough operation-mode=1 ! \
	vvas_xmultisrc kconfig="/opt/xilinx/kv260-smartcam/share/vvas/facedetect/rgba2nv12.json" ! \
	video/x-raw, width=$W, height=$H, format=NV12 ! \
	omxh264enc periodicity-idr=270 target-bitrate=4000 \
	control-rate=low-latency gop-length=60 qp-mode =1 \
	initial-delay=100 cpb-size=200 min-qp=15 max-qp=40 b-frames=0 low-bandwidth=false ! \
	webrtcsink \
	forward-metas="thz" \
	enable-control-data-channel=true \
	run-signalling-server=true \
	run-web-server=true \
	web-server-directory=/opt/gst-plugins-rs/net/webrtc/gstwebrtc-api/dist \
	signalling-server-host=$IP_ADDR \
	web-server-host-addr=http://$IP_ADDR:8080/

#######################################################################################
#scp -r $KV:/home/petalinux/bgr6 ~/Videos/
