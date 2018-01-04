// Includes
#include "INA260.h"

using namespace ina260;

INA260::INA260( I2C *i2cInterfaceIn )
    : m_pI2C( i2cInterfaceIn )
{
    m_address = I2C_ADDRESS_INA260;
}

// --------------------------------------------------------------
// Public API
// --------------------------------------------------------------

void INA260::HardReset()
{
}

void INA260::FullReset()
{
    // Re-enable device
    m_enabled = true;
}

void INA260::Disable()
{
    m_enabled = false;
}

bool INA260::IsEnabled()
{
    return m_enabled;
}

ERetCode INA260::Cmd_ReadSystemStatus(uint16_t *status) {
       return ERetCode::SUCCESS;
}

ERetCode INA260::Cmd_ReadSystemError(uint16_t *error) {
       return ERetCode::SUCCESS;
}  //

ERetCode INA260::Cmd_ReadVoltage( uint32_t *volts ) {
       uint16_t raw_volts = 0;
       ReadRegisterWord( BUS_VOLTAGE, &raw_volts );
       // each unit is 1.25mV
       // return a value in millivolts
       *volts = (BSWAP16(raw_volts) * 125)/100;
       return ERetCode::SUCCESS;
}

ERetCode INA260::Cmd_ReadCurrent( uint32_t *current ) {
       uint16_t raw_amps = 0;
       ReadRegisterWord( CURRENT, &raw_amps );
       // each unit is 1.25mA
       // return a value in milliamps
       *current = (BSWAP16(raw_amps) * 125)/100;
       return ERetCode::SUCCESS;
}

ERetCode INA260::Cmd_ReadPower( uint32_t *watts ) {
       return ERetCode::SUCCESS;
}

ERetCode INA260::Cmd_SetConfig( uint16_t param ) {
       WriteRegisterWord( CONFIGURATION, param );
       return ERetCode::SUCCESS;
}

ERetCode INA260::Cmd_Reset()
{
       return ERetCode::SUCCESS;
}

// --------------------------------------------------------------
// Private Methods
// --------------------------------------------------------------


// I2C call wrappers
i2c::EI2CResult INA260::WriteRegisterWord( uint8_t registerIn, uint16_t dataIn )
{
    return m_pI2C->WriteRegisterWord( m_address, registerIn, dataIn );
}

i2c::EI2CResult INA260::ReadRegisterWord( uint8_t registerIn, uint16_t *dataOut )
{
    return m_pI2C->ReadRegisterWord( m_address, registerIn, dataOut );
}
