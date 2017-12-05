
# This makefile is for Raspberry Pi with Wiring Pi, not AVR, not SAMD.

CPP:=./libraries/ORUtil/orutil.cpp\
./libraries/I2C/src/wiringpi/I2C.cpp\
./libraries/I2C/src/I2C_Shared.cpp\
./OpenROV2x/CCommand.cpp\
./OpenROV2x/CSocket.cpp

INC:=-Ilibraries\
-Ilibraries/I2C/src\
-Ilibraries/BNO055\
-Ilibraries/ORUtil\
-IOpenROV2x

CC=g++

.phony: all
all:
	$(CC) -DWIRINGPI server.cpp ${CPP} -lwiringPi ${INC} -std=gnu++11 -o server

.phony: clean
clean:
	rm -f server
	find . -name '*~' -exec rm -f \{} \;
	find . -name '.o' -exec rm -f \{} \;
