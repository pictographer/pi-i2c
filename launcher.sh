#!/bin/sh
# launcher.sh
#
mkdir -p /home/pi/videos
mkdir -p /home/pi/photos
cd /
cd home/pi
sudo python pin_toggler.py &
cd /
cd home/pi/pi-i2c
sleep 5
sudo ./rovdrv &
cd /
# If this is RPiA wait for RPiB to get its cameras up and running
if [ "$HOSTNAME" != "${HOSTNAME%"B"*}" ]; then
    sleep 30
fi
cd home/pi/openrov-cockpit
sudo PLATFORM=raspberrypi node src/cockpit.js &
cd /
