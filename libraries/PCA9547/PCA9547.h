/* 
 PCA9547 GPIO Expander Library
 By: OpenROV
 Date: September 14, 2016
*/

#pragma once

#include <Arduino.h>
#include <I2C.h>

namespace pca9547
{
    //Unshifted 7-bit I2C address for device
    const uint8_t PCA9547_0_ADDRESS = 0x70; //b1110000
    const uint8_t PCA9547_1_ADDRESS = 0x71; //b1110001
    const uint8_t PCA9547_2_ADDRESS = 0x72; //b1110010

    enum ERetCode : int32_t
    {
        SUCCESS,
        FAILED,
        FAILED_PIN_MODE,
        FAILED_DIGITAL_WRITE,
        TIMED_OUT,
        UNKNOWN
    };

    enum EChan : uint8_t
    {
        CHAN_0,
        CHAN_1,
        CHAN_2,
        CHAN_3,
        CHAN_4,
        CHAN_5,
        CHAN_6,
        CHAN_7,
        NO_CHAN 
    };

    enum PCA9547_SELECT : uint8_t
    {
        NO_CHANNEL = 0x00, //
        CHANNEL_0  = 0x08, //
        CHANNEL_1  = 0x09, //
        CHANNEL_2  = 0x0A, //
        CHANNEL_3  = 0x0B, //
        CHANNEL_4  = 0x0C, //
        CHANNEL_5  = 0x0D, //
        CHANNEL_6  = 0x0E, //
        CHANNEL_7  = 0x0F, //
    };

    class PCA9547
    {
        public:
            PCA9547( I2C* i2cInterfaceIn, uint8_t slaveAddress );

            ERetCode DigitalWrite( uint8_t channel );

            ERetCode Initialize();
            bool IsInitialized() const { return m_isInitialized; };


        private:


            //Private member functions            
            int32_t ReadByte( void );
            int32_t WriteByte( uint8_t dataIn );


            //Private member attributes
            uint8_t m_i2cAddress;
            I2C* m_pI2C;

            bool m_isInitialized = false;

    };
}
