#pragma once

// Driver definitions and helpers
#include <cmath>
#include <Arduino.h>
#include <I2C.h>
#include <orutil.h>

#define BSWAP16(value) (((value & 0x00FF) << 8) | ((value & 0xFF00) >> 8))

namespace ina260
{
    const uint8_t I2C_ADDRESS_INA260  = 0x40;

    enum ERetCode : int32_t
    {
        SUCCESS,
        FAILED,
        TIMED_OUT,
        UNKNOWN
    };

    enum INA260_REGISTER : uint8_t
    {
        CONFIGURATION   = 0x00,
        CURRENT         = 0x01,
        BUS_VOLTAGE     = 0x02,
        POWER           = 0x03,
        MASK_ENABLE     = 0x06,
        ALERT_LIMIT     = 0x07,
        MANUFACTURER_ID = 0xFE,
        DIE_ID          = 0xFF
    };

    class INA260
    {
    public:

        INA260( I2C *i2cInterfaceIn );

        // State machine functions
        void HardReset();
        void FullReset();
        void Disable();

        bool IsEnabled();

        ERetCode Cmd_ReadSystemStatus(uint16_t *status); //
        ERetCode Cmd_ReadSystemError(uint16_t *error);  //

        ERetCode Cmd_ReadVoltage( uint32_t *volts );
        ERetCode Cmd_ReadCurrent( uint32_t *current );
        ERetCode Cmd_ReadPower( uint32_t *watts );
        ERetCode Cmd_SetConfig( uint16_t param );

    private:
        // I2C info
        I2C                     *m_pI2C;
        uint8_t                 m_address;

        // Status
        bool                    m_enabled       = true;

        ERetCode Cmd_Reset();

        // I2C Methods
        i2c::EI2CResult WriteRegisterWord( uint8_t registerIn, uint16_t dataIn );
        i2c::EI2CResult ReadRegisterWord( uint8_t registerIn, uint16_t *dataOut );
    };
}
