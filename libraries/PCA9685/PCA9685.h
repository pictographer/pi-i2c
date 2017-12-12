/* 
 PCA9685 PWM controller
 By: OpenROV
 Date: September 14, 2016
*/

#pragma once

#include <Arduino.h>
#include <I2C.h>

namespace pca9685
{
    //Unshifted 7-bit I2C address for device
    const uint8_t PCA9685_ADDRESS = 0x40; //b1000000

    enum ERetCode : int32_t
    {
        SUCCESS,
        FAILED,
        FAILED_PIN_MODE,
        FAILED_DIGITAL_WRITE,
        TIMED_OUT,
        UNKNOWN
    };

    enum EPin : uint8_t
    {
        LED_0,
        LED_1,
        LED_2,
        LED_3,
        LED_4,
        LED_5,
        LED_6,
        LED_7,
        LED_8,
        LED_9,
        LED_10,
        LED_11,
        LED_12,
        LED_13,
        LED_14,
        LED_15,
        LED_ALL
    };

    class PCA9685
    {
        public:
            PCA9685( I2C* i2cInterfaceIn );

            ERetCode DigitalWrite( uint8_t pin, uint16_t on_value, uint16_t off_value );

            ERetCode Initialize();
            bool IsInitialized() const { return m_isInitialized; };

        private:

            enum class PCA9685_REGISTER : uint8_t
            {
                MODE1 = 0x00, //  mode1 register
                MODE2 = 0x01, //  mode2 register
                SUBADR1 = 0x02, // I2C sub address 1
                SUBADR2 = 0x03, // I2C sub address 2
                SUBADR3 = 0x04, // I2C sub address 3
                ALLCALLADR = 0x05, // LED all call address
                LED0_ON_L  = 0x06, // LED0 output and brightness control byte 0
                LED0_ON_H  = 0x07, // LED0 output and brightness control byte 1
                LED0_OFF_L = 0x08, // LED0 output and brightness control byte 2
                LED0_OFF_H = 0x09, // LED0 output and brightness control byte 3
                LED1_ON_L  = 0x0A, // LED1 output and brightness control byte 0
                LED1_ON_H  = 0x0B, // LED1 output and brightness control byte 1
                LED1_OFF_L = 0x0C, // LED1 output and brightness control byte 2
                LED1_OFF_H = 0x0D, // LED1 output and brightness control byte 3
                LED2_ON_L  = 0x0E, // LED2 output and brightness control byte 0
                LED2_ON_H  = 0x0F, // LED2 output and brightness control byte 1
                LED2_OFF_L = 0x10, // LED2 output and brightness control byte 2
                LED2_OFF_H = 0x11, // LED2 output and brightness control byte 3
                LED3_ON_L  = 0x12, // LED3 output and brightness control byte 0
                LED3_ON_H  = 0x13, // LED3 output and brightness control byte 1
                LED3_OFF_L = 0x14, // LED3 output and brightness control byte 2
                LED3_OFF_H = 0x15, // LED3 output and brightness control byte 3
                LED4_ON_L  = 0x16, // LED4 output and brightness control byte 0
                LED4_ON_H  = 0x17, // LED4 output and brightness control byte 1
                LED4_OFF_L = 0x18, // LED4 output and brightness control byte 2
                LED4_OFF_H = 0x19, // LED4 output and brightness control byte 3
                LED5_ON_L  = 0x1A, // LED5 output and brightness control byte 0
                LED5_ON_H  = 0x1B, // LED5 output and brightness control byte 1
                LED5_OFF_L = 0x1C, // LED5 output and brightness control byte 2
                LED5_OFF_H = 0x1D, // LED5 output and brightness control byte 3
                LED6_ON_L  = 0x1E, // LED6 output and brightness control byte 0
                LED6_ON_H  = 0x1F, // LED6 output and brightness control byte 1
                LED6_OFF_L = 0x20, // LED6 output and brightness control byte 2
                LED6_OFF_H = 0x21, // LED6 output and brightness control byte 3
                LED7_ON_L  = 0x22, // LED7 output and brightness control byte 0
                LED7_ON_H  = 0x23, // LED7 output and brightness control byte 1
                LED7_OFF_L = 0x24, // LED7 output and brightness control byte 2
                LED7_OFF_H = 0x25, // LED7 output and brightness control byte 3
                LED8_ON_L  = 0x26, // LED8 output and brightness control byte 0
                LED8_ON_H  = 0x27, // LED8 output and brightness control byte 1
                LED8_OFF_L = 0x28, // LED8 output and brightness control byte 2
                LED8_OFF_H = 0x29, // LED8 output and brightness control byte 3
                LED9_ON_L  = 0x2A, // LED9 output and brightness control byte 0
                LED9_ON_H  = 0x2B, // LED9 output and brightness control byte 1
                LED9_OFF_L = 0x2C, // LED9 output and brightness control byte 2
                LED9_OFF_H = 0x2D, // LED9 output and brightness control byte 3
                LED10_ON_L = 0x2E, // LED10 output and brightness control byte 0
                LED10_ON_H  = 0x2F, // LED10 output and brightness control byte 1
                LED10_OFF_L = 0x30, // LED10 output and brightness control byte 2
                LED10_OFF_H = 0x31, // LED10 output and brightness control byte 3
                LED11_ON_L  = 0x32, // LED11 output and brightness control byte 0
                LED11_ON_H  = 0x33, // LED11 output and brightness control byte 1
                LED11_OFF_L = 0x34, // LED11 output and brightness control byte 2
                LED11_OFF_H = 0x35, // LED11 output and brightness control byte 3
                LED12_ON_L  = 0x36, // LED12 output and brightness control byte 0
                LED12_ON_H  = 0x37, // LED12 output and brightness control byte 1
                LED12_OFF_L = 0x38, // LED12 output and brightness control byte 2
                LED12_OFF_H = 0x39, // LED12 output and brightness control byte 3
                LED13_ON_L  = 0x3A, // LED13 output and brightness control byte 0
                LED13_ON_H  = 0x3B, // LED13 output and brightness control byte 1
                LED13_OFF_L = 0x3C, // LED13 output and brightness control byte 2
                LED13_OFF_H = 0x3D, // LED13 output and brightness control byte 3
                LED14_ON_L  = 0x3E, // LED14 output and brightness control byte 0
                LED14_ON_H  = 0x3F, // LED14 output and brightness control byte 1
                LED14_OFF_L = 0x40, // LED14 output and brightness control byte 2
                LED14_OFF_H = 0x41, // LED14 output and brightness control byte 3
                LED15_ON_L  = 0x42, // LED15 output and brightness control byte 0
                LED15_ON_H  = 0x43, // LED15 output and brightness control byte 1
                LED15_OFF_L = 0x44, // LED15 output and brightness control byte 2
                LED15_OFF_H = 0x45, // LED15 output and brightness control byte 3
                ALL_LED_ON_L = 0xFA, // ALL LEDn_ON byte 0
                ALL_LED_ON_H = 0xFB, // ALL LEDn_ON byte 1
                ALL_LED_OFF_L = 0xFC, // ALL LEDn_OFF byte 2
                ALL_LED_OFF_H = 0xFD, // ALL LEDn_ON byte 3
                PRESCALE      = 0xFE // Prescaler for PWM output
            };


            //Private member functions            
            int32_t ReadByte( PCA9685_REGISTER addressIn, uint8_t& dataOut );
            int32_t ReadNBytes( PCA9685_REGISTER addressIn, uint8_t* dataOut, uint8_t byteCountIn );
            int32_t WriteByte( PCA9685_REGISTER addressIn, uint8_t dataIn );


            //Private member attributes
            uint8_t m_i2cAddress;
            I2C* m_pI2C;

            uint8_t m_gpioDirection;
            uint8_t m_gpioState;

            bool m_isInitialized = false;

    };
}
