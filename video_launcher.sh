#!/bin/sh
# video_launcher.sh
#
# watch in vlc using network stream 
# http://stir01:8890/?action=stream
# http://stir01:8891/?action=stream
# (RPi hostname is assumed to be stir01)
#
# source snippet: <img src="/?action=stream" />
#
cd /
cd home/pi
export LD_LIBRARY_PATH=/usr/local/lib:/usr/local/lib/mjpg-streamer:${LD_LIBRARY_PATH}
mjpg_streamer -i "input_raspicam.so -x 800 -y 600 -br 70 -quality 10 -framerate 10 -cs 0" -o "output_http.so -w www1 -p 8890" &
sleep 5
mjpg_streamer -i "input_raspicam.so -x 800 -y 600 -br 70 -quality 10 -framerate 10 -cs 1" -o "output_http.so -w www2 -p 8891" &
cd /
