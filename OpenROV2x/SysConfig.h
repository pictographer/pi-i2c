#pragma once

#include "PinDefinitions.h"
#include "CompileOptions.h"

// TODO: The MCU ARCH and Board Type should eventually be passed in solely from the build script.

// ---------------------------------------------------------
// MCU Architecture Selection
// ---------------------------------------------------------

//XXX Porting to new hardware. This file will eventually
// change completely or go away.
//
// #ifndef ARDUINO_ARCH_AVR
//      #error "This sketch is only supported on for ARDUINO_ARCH_AVR!"
// #endif

// Configuration
#define HAS_OROV_CONTROLLERBOARD_25 (1)
#define HAS_STD_LIGHTS (1)
#define HAS_STD_CALIBRATIONLASERS (1)
#define HAS_CAMERASERVO (1)
#define HAS_STD_AUTOPILOT (1)
#define DEADMANSWITCH_ON (0)

// Thrusters configurations  (These appear depricated and replaced by the THRUSTER_CONFIGURATION below)
#define THRUSTER_CONFIG_NONE (0)
#define THRUSTER_CONFIG_2X1 (1)
#define THRUSTER_CONFIG_2Xv2 (2)
#define THRUSTER_CONFIG_v2X1Xv2 (3)
#define THRUSTER_CONFIG_SIMPLIFIED (4)

// Selected Thruster Configuration
//#define THRUSTER_CONFIGURATION THRUSTER_CONFIG_2X1
//#define THRUSTER_CONFIGURATION THRUSTER_CONFIG_2Xv2
//#define THRUSTER_CONFIGURATION THRUSTER_CONFIG_v2X1Xv2
#define THRUSTER_CONFIGURATION THRUSTER_CONFIG_SIMPLIFIED

// ---------------------------------------------------------
// After Market Modules
// ---------------------------------------------------------

// Buoyancy PID loops
#define HAS_BOUYANCY (1)

// Ballast Pump and Valve
#define HAS_BALLAST (1)

// External Lights
#define HAS_EXT_LIGHTS (1)

// AltServo
#define HAS_ALT_SERVO (0)

// MS5611_01BA Depth Sensor
#define HAS_MS5611_01BA (0)

// MS5803_14BA Depth Sensor
#define HAS_MS5803_14BA (0)

// MS5837_30BA Depth Sensor
#define HAS_MS5837_30BA (0)

// 89BSD012BS Depth Sensor
#define HAS_P89BSD012BS (1)

// 86BSD030PA Depth Sensor
#define HAS_P86BSD030PA (1)

// MPU9150
#define HAS_MPU9150 (1)

// BNO055 IMU
#define HAS_BNO055 (1)

// PCA9547 MUX
#define HAS_PCA9547 (1)
