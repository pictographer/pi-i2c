#
# This starts at boot time of RPi through crontab
# It initializes the pins of the RPi and gets the system
# ready to run.
# The script is started by launcher.sh
# The application may override the initial pin settings 
# indicated here.
#
# Import the required module. 
import time
# Get the RPi GPIO module
import RPi.GPIO as GPIO 
# Set the mode of numbering the pins. 
GPIO.setmode(GPIO.BCM) 
# GPIO pin 4 is an input. 
GPIO.setup(4, GPIO.IN) 
input4 = GPIO.input(4);
# GPIO pin 6 is an output. 
GPIO.setup(6, GPIO.OUT) 
# GPIO pin 17 is an output. 
GPIO.setup(17, GPIO.OUT) 
# GPIO pin 18 is an output. 
GPIO.setup(18, GPIO.OUT) 
# GPIO pin 19 is an output. 
GPIO.setup(19, GPIO.OUT) 
# GPIO pin 20 is an output. 
GPIO.setup(20, GPIO.OUT) 
# GPIO pin 21 is an output. 
GPIO.setup(21, GPIO.OUT) 
# GPIO pin 34 is an output
GPIO.setup(34, GPIO.OUT)
# GPIO pin 35 is an output
GPIO.setup(35, GPIO.OUT)
# Initialize 5V_OFF signals 
GPIO.output(20, False) 
GPIO.output(21, False) 
# Initialize the system shutdown pin (STIR_OFF_Rx) to zero (GPIO34)
GPIO.output(34, False) 
# Initialise GPIO6 to high (true) to select the non-USB path
GPIO.output(6, True) 
# Initialise GPIO18 to high (true) to enable I2C
# if RPiA otherwise disable I2C
if (input4 == True) :
    print "RPi A"
    # RPi A runs I2C
    GPIO.output(18, True) 
    # Hold the Air Emergency Balast Valve 'blow' signal HIGH (GPIO35)
    GPIO.output(35, True)
    # Pulse GPIO19 to reset I2C MUXes
    GPIO.output(19, False) 
    time.sleep(0.5)
    GPIO.output(19, True) 
    time.sleep(0.5)
    GPIO.output(19, False) 
else :
    print "RPi B"
    # RPi B does not run I2C
    GPIO.output(18, False) 
    # Hold the Air Emergency Balast Valve 'blow' signal WLOW on the 'other' PI
    GPIO.output(35, False)
print "Toggling watchdog"
while 1: 
#    print "Watch"
     time.sleep(0.250)
     GPIO.output( 17, False) 
     time.sleep(0.250)
     GPIO.output( 17, True)
