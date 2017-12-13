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

    enum DRV10983_REGISTER : uint8_t
    {
        SPEEDCTRL1     = 0x00,
        SPEEDCTRL2     = 0x01,
        DEVCTRL        = 0x02,
        EECTRL         = 0x03,
        STATUSBYTE     = 0x10,
        MOTORSPEED1    = 0x11,
        MOTORSPEED2    = 0x12,
        MOTORPERIOD1   = 0x13,
        MOTORPERIOD2   = 0x14,
        MOTORKT1       = 0x15,
        MOTORKT2       = 0x16,
        IPDPOSITION    = 0x19,
        SUPPLYVOLTAGE  = 0x1A,
        SPEEDCMD       = 0x1B,
        SPEEDCMDBUFFER = 0x1C,
        FAULTCODE      = 0x1E,
        MOTORPARAM1    = 0x20,
        MOTORPARAM2    = 0x21,
        MOTORPARAM3    = 0x22,
        SYSOPT1        = 0x23,
        SYSOPT2        = 0x24,
        SYSOPT3        = 0x25,
        SYSOPT4        = 0x26,
        SYSOPT5        = 0x27,
        SYSOPT6        = 0x28,
        SYSOPT7        = 0x29,
        SYSOPT8        = 0x2A,
        SYSOPT9        = 0x2B
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

        ERetCode Cmd_ReadSystemStatus(uint8_t *status); //
        ERetCode Cmd_ReadSystemError(uint8_t *error);  //

        ERetCode Cmd_SetSpeed( uint16_t speed );

    private:
        // I2C info
        I2C                     *m_pI2C;
        uint8_t                 m_address;

        // Status
        bool                    m_enabled       = true;

        ERetCode Cmd_Reset();

        // I2C Methods
        i2c::EI2CResult WriteRegisterByte( uint8_t registerIn, uint8_t dataIn );
        i2c::EI2CResult ReadRegisterByte( uint8_t registerIn, uint8_t *dataOut );
        i2c::EI2CResult ReadRegisterBytes( uint8_t registerIn, uint8_t *dataOut, uint8_t numBytesIn );
    };
}
