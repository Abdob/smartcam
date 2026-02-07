#######################################################################################
SOURCE1=
SOURCE2=
#######################################################################################
DISPLAY1="kmssink bus-id=fd4a0000.display plane-id=40 sync=false can-scale=false"
DISPLAY2="kmssink driver-name=xlnx plane-id=39 sync=false fullscreen-overlay=true"
RUN=bgr5
FILE="multifilesink location=$RUN/capture%04d.bgr"
#######################################################################################
#R_1920x1080 5 | R_1280x720 10 | R_800x600 24 | R_640x480 30
mkdir -p $RUN
Win=1920
Hin=1080
Wout=1280
Hout=720
gst-launch-1.0 v4l2src device=/dev/video0 io-mode=mmap num-buffers=500 ! \
	h264parse ! video/x-h264,alignment=au ! \
	queue ! omxh264dec low-latency=true ! \
    video/x-raw, width=$Win, height=$Hin, format=NV12 ! \
	queue ! \
	vvas_xmultisrc kconfig="/opt/xilinx/kv260-smartcam/share/vvas/facedetect/resize.json" ! \
	video/x-raw, width=$Wout, height=$Hout, format=BGR ! \
	clockoverlay ! \
	queue ! \
	$DISPLAY2

#######################################################################################
#scp -r $KV:/home/petalinux/bgr3 ~/Videos/
