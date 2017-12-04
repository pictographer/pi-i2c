
# This makefile is for Raspberry Pi with Wiring Pi, not AVR, not SAMD.

CPP:=./libraries/ORUtil/orutil.cpp\
./libraries/BNO055/BNO055.cpp\
./libraries/I2C/src/wiringpi/I2C.cpp\
./libraries/I2C/src/I2C_Shared.cpp\
./OpenROV2x/CCommand.cpp\
./OpenROV2x/CModule.cpp\
./OpenROV2x/CSocket.cpp

INC:=-Ilibraries\
-Ilibraries/I2C/src\
-Ilibraries/BNO055\
-Ilibraries/ORUtil\
-IOpenROV2x

.phony: all
all:
	gcc -DWIRINGPI server.cpp ${CPP} -lwiringPi ${INC} -std=gnu++11 -o server

.phony: clean
	rm server
	find . -name '*~' -exec rm \{} \;
	find . -name '.o' -exec rm \{} \;
