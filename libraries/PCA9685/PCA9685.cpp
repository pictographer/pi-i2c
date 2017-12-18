/* 
 PCA9685 PWM Library
 By: OpenROV
 Date: September 14, 2016
*/

#include "PCA9685.h"

using namespace pca9685;

// map pins to registers
#define GET_ON_L(pin) static_cast<PCA9685_REGISTER>(static_cast<uint8_t>(PCA9685_REGISTER::LED0_ON_L)+(pin*4))
#define GET_ON_H(pin) static_cast<PCA9685_REGISTER>(static_cast<uint8_t>(PCA9685_REGISTER::LED0_ON_H)+(pin*4))
#define GET_OFF_L(pin) static_cast<PCA9685_REGISTER>(static_cast<uint8_t>(PCA9685_REGISTER::LED0_OFF_L)+(pin*4))
#define GET_OFF_H(pin) static_cast<PCA9685_REGISTER>(static_cast<uint8_t>(PCA9685_REGISTER::LED0_OFF_H)+(pin*4))

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

ERetCode PCA9685::Sleep()
{
    auto ret = WriteByte( PCA9685_REGISTER::MODE1, 0x11);
    if( ret != i2c::EI2CResult::RESULT_SUCCESS )
    {
         return ERetCode::FAILED_DIGITAL_WRITE;
    }

    return ERetCode::SUCCESS;
}

ERetCode PCA9685::UnSleep()
{
    // clear the sleep bit
    auto ret = WriteByte( PCA9685_REGISTER::MODE1, 0x01);
    if( ret != i2c::EI2CResult::RESULT_SUCCESS )
    {
         return ERetCode::FAILED_DIGITAL_WRITE;
    }

    // clear the restart bit by writing a one to it
    ret = WriteByte( PCA9685_REGISTER::MODE1, 0x81);
    if( ret != i2c::EI2CResult::RESULT_SUCCESS )
    {
         return ERetCode::FAILED_DIGITAL_WRITE;
    }


    return ERetCode::SUCCESS;
}

ERetCode PCA9685::DigitalWrite( uint8_t pin, uint16_t on_value, uint16_t off_value )
{
    if( pin > LED_ALL )
    {
        return ERetCode::FAILED_DIGITAL_WRITE;
    }

    //Write it
    auto ret = WriteByte( GET_ON_L(pin), on_value&0xFF);
    if( ret != i2c::EI2CResult::RESULT_SUCCESS )
    {
         return ERetCode::FAILED_DIGITAL_WRITE;
    }
    ret = WriteByte( GET_ON_H(pin), (on_value>>8)&0xFF);
    if( ret != i2c::EI2CResult::RESULT_SUCCESS )
    {
         return ERetCode::FAILED_DIGITAL_WRITE;
    }
    ret = WriteByte( GET_OFF_L(pin), off_value&0xFF);
    if( ret != i2c::EI2CResult::RESULT_SUCCESS )
    {
         return ERetCode::FAILED_DIGITAL_WRITE;
    }
    ret = WriteByte( GET_OFF_H(pin), (off_value>>8)&0xFF);
    if( ret != i2c::EI2CResult::RESULT_SUCCESS )
    {
         return ERetCode::FAILED_DIGITAL_WRITE;
    }

    return ERetCode::SUCCESS;
}

ERetCode PCA9685::DigitalWriteHigh( uint8_t pin )
{
    if( pin > LED_ALL )
    {
        return ERetCode::FAILED_DIGITAL_WRITE;
    }

    //Write it
    auto ret = WriteByte( GET_ON_L(pin), 0x00);
    if( ret != i2c::EI2CResult::RESULT_SUCCESS )
    {
         return ERetCode::FAILED_DIGITAL_WRITE;
    }
    ret = WriteByte( GET_ON_H(pin), 0x10);
    if( ret != i2c::EI2CResult::RESULT_SUCCESS )
    {
         return ERetCode::FAILED_DIGITAL_WRITE;
    }
    ret = WriteByte( GET_OFF_L(pin), 0x00);
    if( ret != i2c::EI2CResult::RESULT_SUCCESS )
    {
         return ERetCode::FAILED_DIGITAL_WRITE;
    }
    ret = WriteByte( GET_OFF_H(pin), 0x00);
    if( ret != i2c::EI2CResult::RESULT_SUCCESS )
    {
         return ERetCode::FAILED_DIGITAL_WRITE;
    }

    return ERetCode::SUCCESS;
}

ERetCode PCA9685::DigitalWriteLow( uint8_t pin )
{
    if( pin > LED_ALL )
    {
        return ERetCode::FAILED_DIGITAL_WRITE;
    }

    //Write it
    auto ret = WriteByte( GET_ON_L(pin), 0x00 );
    if( ret != i2c::EI2CResult::RESULT_SUCCESS )
    {
         return ERetCode::FAILED_DIGITAL_WRITE;
    }
    ret = WriteByte( GET_ON_H(pin), 0x00);
    if( ret != i2c::EI2CResult::RESULT_SUCCESS )
    {
         return ERetCode::FAILED_DIGITAL_WRITE;
    }
    ret = WriteByte( GET_OFF_L(pin), 0x00);
    if( ret != i2c::EI2CResult::RESULT_SUCCESS )
    {
         return ERetCode::FAILED_DIGITAL_WRITE;
    }
    ret = WriteByte( GET_OFF_H(pin), 0x10);
    if( ret != i2c::EI2CResult::RESULT_SUCCESS )
    {
         return ERetCode::FAILED_DIGITAL_WRITE;
    }

    return ERetCode::SUCCESS;
}

ERetCode PCA9685::SetPreScale( uint8_t value )
{
    auto ret = WriteByte( PRESCALE, value);
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

