#!/bin/bash
# launcher.sh
#
# Setup photo and video directories
# for stir user but first check 
# if user exists
#
if id "stir"; then
sudo mkdir -p /home/stir/videos
sudo mkdir -p /home/stir/photos
sudo chown stir /home/stir/videos
sudo chown stir /home/stir/photos
sudo chgrp stir /home/stir/videos
sudo chgrp stir /home/stir/photos
else
echo "No user named 'stir' on RPi"
fi
#
# start the system and applications
#
cd /
cd home/pi
sudo python pin_toggler.py &
cd /
cd home/pi/pi-i2c
sleep 5
sudo ./rovdrv &
cd /
# If this is RPiA wait for RPiB to get its cameras up and running
export HOSTNAME=`hostname`
pat="*stir*A*"
if [[ "$HOSTNAME" == $pat ]]; then
    echo "STIR_A: " ${HOSTNAME}
    sleep 30
else
    echo "STIR_B: " ${HOSTNAME}
fi
sleep 5
cd home/pi/openrov-cockpit
sudo PLATFORM=raspberrypi node src/cockpit.js &
cd /
