#######################################################################################
SOURCE1=
SOURCE2=
#######################################################################################
DISPLAY1="kmssink bus-id=fd4a0000.display plane-id=40 sync=false can-scale=false"
DISPLAY2="kmssink driver-name=xlnx plane-id=39 sync=false fullscreen-overlay=true"
RUN=bgr7
FILE="multifilesink location=$RUN/capture%04d.bgr"
#######################################################################################
#R_1920x1080 5 | R_1280x720 10 | R_800x600 24 | R_640x480 30
mkdir -p $RUN
Win=1920
Hin=1080
Wout=1920
Hout=1080
gst-launch-1.0 videotestsrc num-buffers=300 ! \
	"video/x-raw, width=$Win, height=$Hin, format=NV12" ! \
	queue ! \
	vvas_xmultisrc kconfig="/opt/xilinx/kv260-smartcam/share/vvas/facedetect/nv122bgra.json" ! \
	video/x-raw, width=$Wout, height=$Hout, format=RGBA ! \
	queue ! \
	$DISPLAY1

#######################################################################################
#scp -r $KV:/home/petalinux/bgr6 ~/Videos/
