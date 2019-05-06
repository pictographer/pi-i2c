#
# Import the required module. 
import time
import RPi.GPIO as GPIO 
# Set the mode of numbering the pins. 
GPIO.setmode(GPIO.BCM) 
# GPIO pin 6 is an output. 
GPIO.setup(12, GPIO.OUT) 
GPIO.output(12, False) 
# Initialize the system shutdown pin (STIR_OFF_Rx) to zero (GPIO34)
while 1:
    print("on")
    GPIO.output(12, True) 
    time.sleep(0.05)
    print("off")
    GPIO.output(12, False) 
    time.sleep(0.05)
