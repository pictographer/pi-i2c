/* 
 PCA9685 PWM Library
 By: OpenROV
 Date: September 14, 2016
*/

#include "PCA9685.h"

using namespace pca9685;

PCA9685::PCA9685( I2C* i2cInterfaceIn )
    : m_i2cAddress( pca9685::PCA9685_ADDRESS )
    , m_pI2C( i2cInterfaceIn )
{
    
}

ERetCode PCA9685::Initialize()
{
    m_isInitialized = true;

    return ERetCode::SUCCESS;
}

ERetCode PCA9685::DigitalWrite( uint8_t pin, uint16_t on_value, uint16_t off_value )
{
    //Pins 0..7 are r/w capable pins
    if( pin > LED_ALL )
    {
        return ERetCode::FAILED_DIGITAL_WRITE;
    }

    if( value == HIGH )
    {
        //Set the bit to high. (set the bit)
        m_gpioState |= ( 1 << pin );
    }
    else if( value == LOW )
    {
        //Set the bit to high. (clear the bit)
        m_gpioState &= ~( 1 << pin );
    }
    else
    {
        //Don't know what this is
        return ERetCode::FAILED_DIGITAL_WRITE;
    }

    //Write it
    auto ret = WriteByte( PCA9685_REGISTER::OUTPUT_PORT, m_gpioState);
    if( ret != i2c::EI2CResult::RESULT_SUCCESS )
    {
        return ERetCode::FAILED_DIGITAL_WRITE;
    }

    return ERetCode::SUCCESS;

}

/***************************************************************************
    PRIVATE FUNCTIONS
 ***************************************************************************/
int32_t PCA9685::WriteByte( PCA9685_REGISTER addressIn, uint8_t dataIn )
{
	return (int32_t)m_pI2C->WriteRegisterByte( m_i2cAddress, (uint8_t)addressIn, dataIn );
}

int32_t PCA9685::ReadByte( PCA9685_REGISTER addressIn, uint8_t &dataOut )
{
	return (int32_t)m_pI2C->ReadRegisterByte( m_i2cAddress, (uint8_t)addressIn, &dataOut );
}

int32_t PCA9685::ReadNBytes( PCA9685_REGISTER addressIn, uint8_t* dataOut, uint8_t byteCountIn )
{
        return (int32_t)m_pI2C->ReadRegisterBytes( m_i2cAddress, (uint8_t)addressIn, dataOut, byteCountIn );
}

