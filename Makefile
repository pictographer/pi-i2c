.phony: all

all:
	gcc server.cpp -lwiringPi -Ilibraries -Ilibraries/I2C/src -Ilibraries/BNO055 -Ilibraries/ORUtil -std=gnu++11 -o testi2c

