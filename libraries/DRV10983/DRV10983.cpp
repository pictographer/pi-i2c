// Includes
#include "DRV10983.h"

using namespace drv10983;

DRV10983::DRV10983( I2C *i2cInterfaceIn )
    : m_pI2C( i2cInterfaceIn )
{
    m_address = I2C_ADDRESS_A;
}

// --------------------------------------------------------------
// Public API
// --------------------------------------------------------------

void DRV10983::Tick()
{
}

void DRV10983::HardReset()
{
}

void DRV10983::FullReset()
{
    // Re-enable device
    m_enabled = true;
}

void DRV10983::Disable()
{
    m_enabled = false;
}

bool DRV10983::IsEnabled()
{
    return m_enabled;
}

// --------------------------------------------------------------
// Private Methods
// --------------------------------------------------------------

ERetCode DRV10983::Cmd_Reset()
{
        return ERetCode::SUCCESS;
}

ERetCode DRV10983::Cmd_ReadSystemStatus(uint8_t *status)
{
	return ERetCode::SUCCESS;
}

ERetCode DRV10983::Cmd_ReadSystemError(uint8_t *error)
{
	return ERetCode::SUCCESS;
}

ERetCode DRV10983::Cmd_SetSpeed(uint16_t speed)
{
        WriteRegisterByte( DRV10983_REGISTER::SPEEDCTRL1, speed&0xFF );
        WriteRegisterByte( DRV10983_REGISTER::SPEEDCTRL2, ((0x80)|(speed>>8)&0x01) );
      
	return ERetCode::SUCCESS;
}

// I2C call wrappers
i2c::EI2CResult DRV10983::WriteRegisterByte( uint8_t registerIn, uint8_t dataIn )
{
    return m_pI2C->WriteRegisterByte( m_address, registerIn, dataIn );
}

i2c::EI2CResult DRV10983::ReadRegisterByte( uint8_t registerIn, uint8_t *dataOut )
{
    return m_pI2C->ReadRegisterByte( m_address, registerIn, dataOut );
}

i2c::EI2CResult DRV10983::ReadRegisterBytes( uint8_t registerIn, uint8_t *dataOut, uint8_t numBytesIn )
{
    return m_pI2C->ReadRegisterBytes( m_address, registerIn, dataOut, numBytesIn );
}
