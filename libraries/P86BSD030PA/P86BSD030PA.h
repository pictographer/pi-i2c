#pragma once

// Driver definitions and helpers
#include "P86BSD030PA_Def.h"

namespace p86bsd030pa
{
    class P86BSD030PA
    {
    public:
        // Attributes
        TData m_data;

        // Methods
        P86BSD030PA( I2C *i2cInterfaceIn );

        // State machine functions
        void Tick();
        void HardReset();
        void FullReset();
        void Disable();

        bool IsEnabled();

        bool GetLock();
        void FreeLock();

        // Configuration
        EResult SetOversampleRate( EOversampleRate rateIn );
        EResult SetWaterType( EWaterType typeIn );

        uint32_t GetUpdatePeriod();

        // Statistics
        uint32_t GetResultCount( EResult resultTypeIn );
        void ClearResultCount( EResult resultTypeIn );

    private:
        // I2C info
        I2C                     *m_pI2C;
        uint8_t                 m_address;

        // Configuration
        EOversampleRate         m_osr       = EOversampleRate::OSR_256_SAMPLES;
        EWaterType              m_waterType = EWaterType::FRESH;
        float                   m_waterMod  = kWaterModFresh;
        uint8_t                 m_conversionTime_ms = kConversionTime_ms;

        // State management
        bool                    m_enabled   = true;
        EState                  m_state     = EState::UNINITIALIZED;
        TTransitionDelay        m_delay;
        //TStateCallback          m_pStateMethods[ EState::_STATE_COUNT ] = { nullptr };

        // Result statistics
        orutil::TResultCount<EResult::_RESULT_COUNT, EResult::RESULT_SUCCESS> m_results;
    
        // Computation parameters
        uint16_t    m_coeffs[8] = {0};  // unsigned 16-bit integer (0-65535)

        uint32_t    m_D1        = 0;    // Digital pressure value
        uint32_t    m_D2        = 0;    // Digital temperature value
        
        int32_t     m_dT        = 0;    // Difference between actual and reference temperature
        int32_t     m_TEMP      = 0;    // Actual temperature
        
        int64_t     m_OFF       = 0;    // Offset at actual temperature
        int64_t     m_SENS      = 0;    // Sensitivity at actual temperature
        
        int64_t     m_Ti        = 0;    // Second order temperature component       
        int64_t     m_OFFi      = 0;    // Second order offset component            
        int64_t     m_SENSi     = 0;    // Second order sens component             
        
        int64_t     m_OFF2      = 0;    // Second order final off 
        int64_t     m_SENS2     = 0;    // Second order final sens
        float       m_TEMP2     = 0;    // Second order final temp
        
        float       m_P         = 0;    // Temperature compensated pressure
        
        // Bytes to hold the results from I2C communications with the sensor
        uint8_t     m_highByte  = 0;
        uint8_t     m_midByte   = 0;
        uint8_t     m_lowByte   = 0;

        // Methods
        void Transition( EState nextState );
        void DelayedTransition( EState nextState, uint32_t millisIn );
        float CalculateDepth( float pressure, float temp );

        EResult Cmd_Reset();
        EResult Cmd_ReadCalibrationData();
        EResult Cmd_StartTempConversion();
        EResult Cmd_StartPresConversion();
        EResult Cmd_ReadTemperature();
        EResult Cmd_ReadPressure();

        uint8_t CalculateCRC4();
        void ProcessData();

        // I2C Methods
        i2c::EI2CResult WriteByte( uint8_t registerIn );
        i2c::EI2CResult ReadByte( uint8_t *dataOut );
        i2c::EI2CResult ReadBytes( uint8_t *dataOut, uint8_t numBytesIn );
    };
}
