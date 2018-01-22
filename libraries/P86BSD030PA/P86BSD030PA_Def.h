#pragma once

#include <Arduino.h>
#include <I2C.h>
#include <orutil.h>

namespace p86bsd030pa
{
    // I2C Slave Address
    constexpr uint8_t I2C_ADDRESS           = 0x28;

    // Commands
    constexpr uint8_t CMD_ADC_READ          = 0x00;         // 24 bit result
    constexpr uint8_t CMD_RESET             = 0x1E;
    constexpr uint8_t CMD_PRES_CONV_BASE    = 0x40;         // OSR (+2 step) (256, 512, 1024, 2048, 4096)
    constexpr uint8_t CMD_TEMP_CONV_BASE    = 0x50;         // OSR (+2 step) (256, 512, 1024, 2048, 4096)
    constexpr uint8_t CMD_PROM_READ_BASE    = 0xA0;         // (+2 step) coefficient index (16 bit result)

    constexpr float kWaterModFresh          = 1.019716f;
    constexpr float kWaterModSalt           = 0.9945f;

    constexpr uint32_t kRetryDelay_ms       = 1000;
    constexpr uint32_t kResetDelay_ms       = 10;

    constexpr uint8_t kConversionTime_ms    = 10;

    // Computation constants
    constexpr int32_t POW_2_7  = 128;           // 2^7
    constexpr int32_t POW_2_8  = 256;           // 2^8
    constexpr int32_t POW_2_15 = 32768;         // 2^15
    constexpr int32_t POW_2_16 = 65536;         // 2^16
    constexpr int32_t POW_2_21 = 2097152;       // 2^21
    constexpr int32_t POW_2_23 = 8388608;       // 2^23
    constexpr int32_t POW_2_31 = 2147483648;    // 2^31

    //typedef void (*TStateCallback)();

    enum class EAddress
    {
        ADDRESS
    };

    enum class EWaterType
    {
        FRESH   = 0,
        SALT    = 1
    };
    
    enum EState : uint8_t
    {
        UNINITIALIZED,
        DISABLED,
        DELAY,
        READING_CALIB_DATA,
        CONVERTING_PRESSURE,
        CONVERTING_TEMPERATURE,
        PROCESSING_DATA,

        _STATE_COUNT
    };

    enum EResult : uint32_t
    {
        RESULT_SUCCESS,
        
        RESULT_ERR_HARD_RESET,
        RESULT_ERR_FAILED_SEQUENCE,
        RESULT_ERR_I2C_TRANSACTION,
        RESULT_ERR_CRC_MISMATCH,

        _RESULT_COUNT
    };

    // Data structure
    struct TData
    {
        public:
            float temperature_c = 0.0f;
            float pressure_mbar = 0.0f;
            float depth_m       = 0.0f;

            bool SampleAvailable()
            {
                if( isAvailable )
                {
                    isAvailable = false;
                    return true;
                }
                else
                {
                    return false;
                }
            }

        private:
            void Update( float tempIn, float presIn, float depth )
            {
                temperature_c   = tempIn;
                pressure_mbar   = presIn;
                depth_m         = depth;
                isAvailable     = true;
            }

            bool isAvailable = false;

        friend class P86BSD030PA;
    };

    // Delay state info
    struct TTransitionDelay
    {
        orutil::CTimer  timer;
        EState          nextState;
        uint32_t        delayTime_ms;
    };

    // ----------
    // OSR
    enum class EOversampleRate : uint8_t
    {
        OSR_256_SAMPLES = 0,
        OSR_512_SAMPLES,
        OSR_1024_SAMPLES,
        OSR_2048_SAMPLES,
        OSR_4096_SAMPLES,
        _OSR_COUNT
    };

    struct TOversampleInfo
    {
        uint8_t commandMod;
        uint8_t conversionTime_ms;
    };

    // Table of command modifiers and conversion times for each OSR
    constexpr TOversampleInfo kOSRInfo[ (uint8_t)EOversampleRate::_OSR_COUNT ] =
    { 
        { 0x00, 1 },    // 256
        { 0x02, 2 },    // 512
        { 0x04, 3 },    // 1024
        { 0x06, 5 },    // 2048
        { 0x08, 10 },   // 4096
    };
}
