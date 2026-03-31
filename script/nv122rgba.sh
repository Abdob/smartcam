#####################################################################################
DISPLAY1="kmssink plane-id=41 sync=false can-scale=false"
DISPLAY2="kmssink driver-name=xlnx plane-id=39 sync=false fullscreen-overlay=true"
#####################################################################################
#R_1920x1080 5 | R_1280x720 10 | R_800x600 24 | R_640x480 30
W=1920
H=1080
gst-launch-1.0 videotestsrc ! \
	video/x-raw, width=$W, height=$H, format=RGBA ! \
	thzmeta ! \
	thzoverlay timeout=3000 ! \
	clockoverlay ! \
	queue ! \
	$DISPLAY1

#######################################################################################
