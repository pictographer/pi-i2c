#pragma once


#include <Arduino.h>
#include <I2C.h>
#include <orutil.h>

namespace drv10983
{
	// I2C Slave Address
        const uint8_t I2C_ADDRESS_A                            = 0x52;

	enum ERegister : uint8_t
	{
		SPEED_CTRL1                                     = 0x00,
		SPEED_CTRL2                                     = 0x01,
		DEV_CTRL                                        = 0x02,
		EE_CTRL                                         = 0x03,
		STATUS                                          = 0x10,
                 
                MOTOR_SPEED1                                    = 0x11,
                MOTOR_SPEED2                                    = 0x12,
                MOTOR_PERIOD1                                   = 0x13,
                MOTOR_PERIOD2                                   = 0x14,
                MOTOR_KT1                                       = 0x15,
                MOTOR_KT2                                       = 0x16,

                IPD_POSITION                                    = 0x19,
                SUPPLY_VOLTAGE                                  = 0x1A,
                SPEED_CMD                                       = 0x1B,
                SPD_CMD_BUFFER                                  = 0x1C,

                FAULT_CODE                                      = 0x1E,
                //
                // Begin: EEPROM ADDRESSES
                //
                MOTOR_PARAM1                                    = 0x20,
                MOTOR_PARAM2                                    = 0x21,
                MOTOR_PARAM3                                    = 0x22,

                SYS_OPT1                                        = 0x23,
                SYS_OPT2                                        = 0x24,
                SYS_OPT3                                        = 0x25,
                SYS_OPT4                                        = 0x26,
                SYS_OPT5                                        = 0x27,
                SYS_OPT6                                        = 0x28,
                SYS_OPT7                                        = 0x29,
                SYS_OPT8                                        = 0x2A,
                SYS_OPT9                                        = 0x2B,
                //
                // End: EEPROM ADDRESSES
                //
	};
};
