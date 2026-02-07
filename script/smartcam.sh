#stride-align=256
#####################################################################################
DISPLAY1="kmssink bus-id=fd4a0000.display plane-id=40 sync=false can-scale=false"
DISPLAY2="kmssink driver-name=xlnx plane-id=39 sync=false fullscreen-overlay=true"
#####################################################################################
#R_1920x1080 5 | R_1280x720 10 | R_800x600 24 | R_640x480 30
W=1920
H=1080
gst-launch-1.0 v4l2src device=/dev/video0 io-mode=mmap num-buffers=300 ! \
	h264parse ! \
	"video/x-h264,alignment=au" ! \
	queue ! \
	omxh264dec low-latency=true ! \
	"video/x-raw, width=$W, height=$H" ! \
	tee name=t ! \
	queue ! \
	vvas_xmultisrc kconfig="/opt/xilinx/kv260-smartcam/share/vvas/facedetect/preprocess.json" ! \
	queue ! \
	vvas_xfilter kernels-config="/opt/xilinx/kv260-smartcam/share/vvas/facedetect/aiinference.json" ! \
	ima.sink_master vvas_xmetaaffixer name=ima ima.src_master ! fakesink \
	t. ! queue max-size-buffers=1 leaky=2 ! ima.sink_slave_0 ima.src_slave_0 ! queue ! \
	vvas_xfilter kernels-config="/opt/xilinx/kv260-smartcam/share/vvas/facedetect/drawresult.json" \
	! queue ! \
	kmssink driver-name=xlnx plane-id=39 sync=false fullscreen-overlay=true -v
