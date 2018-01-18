#!/bin/sh
#
# test if WiFi is still alive
#
ping -c4 192.168.1.254 > /dev/null

if [ $? != 0 ]
then
  echo "No network connection, restarting wlan0"
  /sbin/ifdown 'wlan0'
  sleep 5
  /sbin/ifup --force 'wlan0'
fi
