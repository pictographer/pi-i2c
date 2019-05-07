#!/bin/sh
export LD_LIBRARY_PATH=/usr/local/lib/mjpg-streamer:/usr/local/lib
mjpg_streamer -i "input_raspicam.so -x 1280 -y 720 -br 70 -quality 10 -framerate 10 -cs 1" -o "output_ws.so -p 8091" -o "output_file.so -f /home/pi -m video.0.test.mjpeg"  -o "output_http.so -w /home/pi/www1 -p 9091"
