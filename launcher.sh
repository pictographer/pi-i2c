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
#If this is RPiA wait for RPiB to get its cameras up and running
if [ "$HOSTNAME" contains "A" ];
then
    sleep 15
fi
cd /
cd home/pi/openrov-cockpit
sleep 5
sudo PLATFORM=raspberrypi node src/cockpit.js &
cd /
