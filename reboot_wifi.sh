#!/bin/sh
#
# test if WiFi is still alive
#
export GATEWAY=`route -n | grep 'UG[ \t]' | awk '{print $2}'`
echo "ping -c4 ${GATEWAY} > /dev/null"
ping -c4 ${GATEWAY} > /dev/null

if [ $? != 0 ]
then
  echo "No network connection, restarting wlan0"
  /sbin/ifdown 'wlan0'
  sleep 5
  /sbin/ifup --force 'wlan0'
fi

if [ -f /home/pi/pi-i2c/updater.py ]
then
  echo "Starting update..."
  python /home/pi/pi-i2c/updater.py
fi
