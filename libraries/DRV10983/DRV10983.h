#pragma once

// Driver definitions and helpers
#include "DRV10983_Def.h"

namespace drv10983
{
    enum ERetCode : int32_t
    {
        SUCCESS,
        FAILED,
        FAILED_PIN_MODE,
        FAILED_DIGITAL_WRITE,
        TIMED_OUT,
        UNKNOWN
    };

    class DRV10983
    {
    public:

        DRV10983( I2C *i2cInterfaceIn );

        // State machine functions
        void Tick();
        void HardReset();
        void FullReset();
        void Disable();

        bool IsEnabled();

        uint32_t GetUpdatePeriod();

    private:
        // I2C info
        I2C                     *m_pI2C;
        uint8_t                 m_address;

        // Status
        bool                    m_enabled       = true;

        ERetCode Cmd_Reset();

        ERetCode Cmd_ReadSystemStatus(uint8_t *status); //
        ERetCode Cmd_ReadSystemError(uint8_t *error);  //

        ERetCode Cmd_SetSpeed( uint16_t speed );

        // I2C Methods
        i2c::EI2CResult WriteRegisterByte( uint8_t registerIn, uint8_t dataIn );
        i2c::EI2CResult ReadRegisterByte( uint8_t registerIn, uint8_t *dataOut );
        i2c::EI2CResult ReadRegisterBytes( uint8_t registerIn, uint8_t *dataOut, uint8_t numBytesIn );
    };
}
