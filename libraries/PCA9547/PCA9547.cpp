/* 
 PCA9547 GPIO Expander Library
 By: OpenROV
 Date: September 14, 2016
*/

#include "PCA9547.h"

using namespace pca9547;

PCA9547::PCA9547( I2C* i2cInterfaceIn, uint8_t slaveAddress )
    : m_i2cAddress( slaveAddress )
    , m_pI2C( i2cInterfaceIn )
{
    
}

ERetCode PCA9547::Initialize()
{

    m_isInitialized = true;

    return ERetCode::SUCCESS;
}

ERetCode PCA9547::DigitalWrite( uint8_t channel )
{
    //Pins 0..7 are r/w capable pins
    if( channel > NO_CHAN )
    {
        return ERetCode::FAILED_DIGITAL_WRITE;
    }

    //Write it
    auto ret = WriteByte( channel );
    if( ret != i2c::EI2CResult::RESULT_SUCCESS )
    {
        return ERetCode::FAILED_DIGITAL_WRITE;
    }

    return ERetCode::SUCCESS;

}

/***************************************************************************
    PRIVATE FUNCTIONS
 ***************************************************************************/
int32_t PCA9547::WriteByte( uint8_t dataIn )
{
	return (int32_t)m_pI2C->WriteByte( m_i2cAddress, dataIn );
}

int32_t PCA9547::ReadByte( void )
{
	return (int32_t) 0; //m_pI2C->ReadByte( m_i2cAddress );
}
