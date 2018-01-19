
# This makefile is for Raspberry Pi with Wiring Pi, not AVR, not SAMD.

CPP:=./OpenROV2x/CAutopilot_STD.cpp\
./OpenROV2x/CLights.cpp\
./OpenROV2x/CCalibrationLaser.cpp\
./OpenROV2x/CMPU9150.cpp\
./OpenROV2x/CModule.cpp\
./OpenROV2x/CMotor.cpp\
./OpenROV2x/NDataManager.cpp\
./OpenROV2x/NCommManager.cpp\
./OpenROV2x/NModuleManager.cpp\
./OpenROV2x/CSocket.cpp\
./OpenROV2x/CP86BSD030PA.cpp\
./OpenROV2x/CP89BSD012BS.cpp\
./OpenROV2x/CMS5611_01BA.cpp\
./OpenROV2x/CMS5803_14BA.cpp\
./OpenROV2x/NArduinoManager.cpp\
./OpenROV2x/CCommand.cpp\
./OpenROV2x/CBNO055.cpp\
./OpenROV2x/CExternalLights.cpp\
./OpenROV2x/NVehicleManager.cpp\
./OpenROV2x/CMS5837_30BA.cpp\
./OpenROV2x/CPin.cpp\
./OpenROV2x/CMuxes.cpp\
./OpenROV2x/CBallast.cpp\
./OpenROV2x/CCameraServo.cpp\
./OpenROV2x/CControllerBoard.cpp\
./OpenROV2x/CThrusters_2X1.cpp\
./OpenROV2x/CThrusters_2Xv2.cpp\
./OpenROV2x/CThrusters_v2X1Xv2.cpp\
./OpenROV2x/CThrusters_Simplified.cpp\
./libraries/ORUtil/orutil.cpp\
./libraries/P86BSD030PA/P86BSD030PA.cpp \
./libraries/P89BSD012BS/P89BSD012BS.cpp \
./libraries/MS5837_30BA/MS5837_30BA.cpp\
./libraries/MS5803_14BA/MS5803_14BA.cpp\
./libraries/MS5611_01BA/MS5611_01BA.cpp\
./libraries/MPU9150/MPU9150_Vector3.cpp\
./libraries/MPU9150/MPU9150.cpp\
./libraries/MPU9150/MPU9150_Calibration.cpp\
./libraries/MPU9150/MPU9150_DMPDriver.cpp\
./libraries/MPU9150/MPU9150_Quaternion.cpp\
./libraries/MPU9150/MPU9150_DriverLayer.cpp\
./libraries/BQ34Z100/BQ34Z100.cpp\
./libraries/BNO055/BNO055.cpp\
./libraries/PCA9547/PCA9547.cpp\
./libraries/PCA9685/PCA9685.cpp\
./libraries/INA260/INA260.cpp\
./libraries/DRV10983/DRV10983.cpp\
./libraries/MPL3115A2/MPL3115A2.cpp\
./libraries/PCA9539/PCA9539.cpp\
./libraries/I2C/src/I2C_Shared.cpp\
./libraries/I2C/src/samd/I2C.cpp\
./libraries/I2C/src/wiringpi/I2C.cpp\
OpenROV2x.cpp

BROKEN:=\
./libraries/Servo/src/avr/Servo.cpp\
./libraries/Servo/src/samd/Servo.cpp\
./libraries/I2C/src/avr/I2C.cpp\
./libraries/I2C/src/avr/I2C_Driver.cpp

INC:=-Ilibraries\
-Ilibraries/BNO055\
-Ilibraries/BQ34Z100\
-Ilibraries/I2C/src\
-Ilibraries/MPL3115A2\
-Ilibraries/P86BSD030PA\
-Ilibraries/P89BSD012BS\
-Ilibraries/MPU9150\
-Ilibraries/MS5611_01BA\
-Ilibraries/MS5803_14BA\
-Ilibraries/MS5837_30BA\
-Ilibraries/ORUtil\
-Ilibraries/PCA9539\
-Ilibraries/PCA9547\
-Ilibraries/PCA9685\
-Ilibraries/DRV10983\
-Ilibraries/INA260\
-Ilibraries/Servo\
-IOpenROV2x

CC=g++

.phony: all
all:
	$(CC) -g -DWIRINGPI ${CPP} -lwiringPi ${INC} -std=gnu++11 -o rovdrv

.phony: clean
clean:
	rm -f rovdrv
	find . -name '*~' -exec rm -f \{} \;
	find . -name '.o' -exec rm -f \{} \;
