// Includes
#include "P89BSD012BS.h"
#include <assert.h>

using namespace p89bsd012bs;

P89BSD012BS::P89BSD012BS( I2C *i2cInterfaceIn )
    : m_pI2C( i2cInterfaceIn )
    , m_address( I2C_ADDRESS )
{
}

// --------------------------------------------------------------
// Public API
// --------------------------------------------------------------

void P89BSD012BS::Tick()
{
    // TODO: Clever way to do this with function pointers and lambdas?
    switch( m_state )
    {
        case EState::DELAY:
        {
            // Do nothing until requested time has elapsed
            if( m_delay.timer.HasElapsed( m_delay.delayTime_ms ) )
            {
                // Move to next state
                Transition( m_delay.nextState );
            }

            break;
        }

        // -----------------------------
        // Active operating conditions
        // -----------------------------
        case EState::CONVERTING_PRESSURE:
        {
            // Start a pressure conversion
            if( Cmd_StartPresConversion() )
            {
                // Failure, start new conversion sequence
                DelayedTransition( EState::CONVERTING_PRESSURE, kOSRInfo[ static_cast<uint8_t>( m_osr ) ].conversionTime_ms  );
                m_results.AddResult( EResult::RESULT_ERR_FAILED_SEQUENCE );
            }
            else
            {
                // Success, next read the value and start converting pressure
                DelayedTransition( EState::CONVERTING_TEMPERATURE, kOSRInfo[ static_cast<uint8_t>( m_osr ) ].conversionTime_ms  );
            }

            break;
        };

        case EState::CONVERTING_TEMPERATURE:
        {
            // Read pressure
            if( Cmd_ReadPressure() )
            {
                // Failure, start new conversion sequence
                DelayedTransition( EState::CONVERTING_PRESSURE, kOSRInfo[ static_cast<uint8_t>( m_osr ) ].conversionTime_ms  );
                m_results.AddResult( EResult::RESULT_ERR_FAILED_SEQUENCE );
            }
            else
            {
                // Start temp conversion
                if( Cmd_StartTempConversion() )
                {
                    // Failure, start new conversion sequence
                    DelayedTransition( EState::CONVERTING_PRESSURE, kOSRInfo[ static_cast<uint8_t>( m_osr ) ].conversionTime_ms  );
                    m_results.AddResult( EResult::RESULT_ERR_FAILED_SEQUENCE );
                }
                else
                {
                    // Success
                    DelayedTransition( EState::PROCESSING_DATA, kOSRInfo[ static_cast<uint8_t>( m_osr ) ].conversionTime_ms  );
                }
            }

            break;
        };

        case EState::PROCESSING_DATA:
        {
            // Read temperature
            if( Cmd_ReadTemperature() )
            {
                // Failure, start new conversion sequence
                Transition( EState::CONVERTING_PRESSURE );
                m_results.AddResult( EResult::RESULT_ERR_FAILED_SEQUENCE );
            }
            else
            {
                // Calculate depth and announce available data sample
                ProcessData();

                // Return to for the next read
                Transition( EState::CONVERTING_PRESSURE );
            }

            break;
        };

        // ------------------------
        // Initialization routines
        // ------------------------

        case EState::UNINITIALIZED:
        {
            // Attempt to reset
            if( Cmd_Reset() )
            {
                // Failure
                HardReset();
            }
            else
            {
                // Sucess
                DelayedTransition( EState::READING_CALIB_DATA, kResetDelay_ms );
            }

            break;
        }

        case EState::READING_CALIB_DATA:
        {
            // Fetch calibration coefficients
            if( Cmd_ReadCalibrationData() )
            {
                // Failure
                HardReset();
            }
            else
            {
                // Success
                Transition( EState::CONVERTING_PRESSURE );
            }

            break;
        };

        case EState::DISABLED:
        default:
        {
            // Do nothing
            break;
        }
    }
}

void P89BSD012BS::HardReset()
{
    // Perform delayed transition to uninitialized
    DelayedTransition( EState::UNINITIALIZED, kRetryDelay_ms );

    // Increment hard reset counter
    m_results.AddResult( EResult::RESULT_ERR_HARD_RESET );

    // Clear the sequence error counter
    ClearResultCount( EResult::RESULT_ERR_FAILED_SEQUENCE );
}

void P89BSD012BS::FullReset()
{
    // Perform delayed transition to uninitialized
    DelayedTransition( EState::UNINITIALIZED, kRetryDelay_ms );

    // Clear all results
    m_results.Clear();

    // Re-enable device
    m_enabled = true;
}

uint32_t P89BSD012BS::GetResultCount( EResult resultTypeIn )
{
    return m_results.GetResultCount( resultTypeIn );
}

void P89BSD012BS::ClearResultCount( EResult resultTypeIn )
{
    m_results.ClearResult( resultTypeIn );
}

void P89BSD012BS::Disable()
{
    // Disable device entirely
    Transition( EState::DISABLED );

    m_enabled = false;
}

bool P89BSD012BS::IsEnabled()
{
    return m_enabled;
}

bool P89BSD012BS::GetLock()
{
    return m_pI2C->LockAddress( m_address );
}

void P89BSD012BS::FreeLock()
{
    m_pI2C->FreeAddress( m_address );
}

EResult P89BSD012BS::SetOversampleRate( EOversampleRate rateIn )
{
    // Change the oversampling rate
    m_osr = rateIn;
 
    // Reset sensor, since we might be in the middle of a conversion.
    Transition( EState::UNINITIALIZED );

    return EResult::RESULT_SUCCESS;
}

EResult P89BSD012BS::SetWaterType( EWaterType typeIn )
{
    // Update the water modifier. No need to adjust state machine.
    m_waterMod = ( ( typeIn == EWaterType::FRESH ) ? kWaterModFresh : kWaterModSalt );

    return EResult::RESULT_SUCCESS;
}

uint32_t P89BSD012BS::GetUpdatePeriod()
{
    // Two conversion periods
    return 2 * m_conversionTime_ms;
}

// --------------------------------------------------------------
// Private Methods
// --------------------------------------------------------------

void P89BSD012BS::Transition( EState stateIn )
{
    // Set new state
    m_state = stateIn;
}

void P89BSD012BS::DelayedTransition( EState nextState, uint32_t millisIn )
{
    // Set delay info
    m_state                 = EState::DELAY;
    m_delay.nextState       = nextState;
    m_delay.delayTime_ms    = millisIn;
    m_delay.timer.Reset(); 
}

EResult P89BSD012BS::Cmd_Reset()
{
    // This device has no reset command
    return EResult::RESULT_SUCCESS;
}

EResult P89BSD012BS::Cmd_ReadCalibrationData()
{
    uint8_t coeffs[ 2 ];
 
    // Read sensor coefficients
    for( uint8_t i = 0; i < 10; ++i )
    {
        i2c::EI2CResult ret = ReadRegisterBytes( CMD_PROM_READ_BASE + ( i * 2 ), coeffs, 2 );
 
 	if( ret )
 	{
            // I2C Failure
            return (EResult)m_results.AddResult( EResult::RESULT_ERR_I2C_TRANSACTION );
 	}
 
        // Combine high and low bytes
        m_coeffs[i] = ( ( ( uint16_t )coeffs[ 0 ] << 8 ) | coeffs[ 1 ] );
    }
 
    uint8_t readCRC = ( m_coeffs[ 0 ] >> 12 );
 
   // Compare read CRC4 to calculated CRC4
   if( readCRC == CalculateCRC4() )
   {
 	return EResult::RESULT_SUCCESS;
   }
   else
   {
 	return (EResult)m_results.AddResult( EResult::RESULT_ERR_CRC_MISMATCH );
   }
   return EResult::RESULT_SUCCESS;
}

EResult P89BSD012BS::Cmd_StartPresConversion()
{

    // reading the device initiates a measurement
    if( WriteByte( CMD_PRES_CONV_BASE + kOSRInfo[ static_cast<uint8_t>( m_osr ) ].commandMod ) )
    {
        // I2C Failure
        return (EResult)m_results.AddResult( EResult::RESULT_ERR_I2C_TRANSACTION );
    }
    else
    {
        // Success
        return EResult::RESULT_SUCCESS;
    }
}

EResult P89BSD012BS::Cmd_StartTempConversion()
{

    // reading the device initiates a measurement
    if( WriteByte( CMD_TEMP_CONV_BASE + kOSRInfo[ static_cast<uint8_t>( m_osr ) ].commandMod ) )
    {
        // I2C Failure
        return (EResult)m_results.AddResult( EResult::RESULT_ERR_I2C_TRANSACTION );
    }
    else
    {
        // Success
        return EResult::RESULT_SUCCESS;
    }
}

EResult P89BSD012BS::Cmd_ReadPressure()
{
    uint8_t bytes[ 3 ];

    i2c::EI2CResult ret = ReadRegisterBytes( CMD_ADC_READ, bytes, 3 );
    if( ret )
    {
        // I2C Failure
        return (EResult)m_results.AddResult( EResult::RESULT_ERR_I2C_TRANSACTION );
    }

    // Combine the bytes into one integer for the final result
    m_D1 = ( ( uint32_t )bytes[ 0 ] << 16 ) + ( ( uint32_t )bytes[ 1 ] << 8 ) + ( uint32_t )bytes[ 2 ];

    return EResult::RESULT_SUCCESS;
}

EResult P89BSD012BS::Cmd_ReadTemperature()
{
    uint8_t bytes[ 3 ];

    i2c::EI2CResult ret = ReadRegisterBytes( CMD_ADC_READ, bytes, 3 );
    if( ret )
    {
        // I2C Failure
        return (EResult)m_results.AddResult( EResult::RESULT_ERR_I2C_TRANSACTION );
    }

    // Combine the bytes into one integer for the final result
    m_D2 = ( ( uint32_t )bytes[ 0 ] << 16 ) + ( ( uint32_t )bytes[ 1 ] << 8 ) + ( uint32_t )bytes[ 2 ];

    return EResult::RESULT_SUCCESS;
}

uint8_t P89BSD012BS::CalculateCRC4()
{
    int cnt;            // simple counter
    uint32_t n_rem = 0; // crc remainder
    uint8_t n_bit;
    
    // Replace the CRC byte with 0
    m_coeffs[0] = ((m_coeffs[0]) & 0x0FFF); 

    // Leftover value from the MS5803 series, set to 0
    m_coeffs[7] = 0;
    
    // Loop through each byte in the coefficients
    for( cnt = 0; cnt < 16; ++cnt )
    { 
        // Choose LSB or MSB
        if( ( cnt % 2 ) == 1 ) 
        {
            n_rem ^= (uint16_t)( ( m_coeffs[ cnt >> 1 ] ) & 0x00FF );
        }
        else
        {
            n_rem ^= (uint16_t)( m_coeffs[ cnt >> 1 ] >> 8 );
        }
            
        for( n_bit = 8; n_bit > 0; --n_bit )
        {
            if( n_rem & 0x8000 )
            {
                n_rem = ( n_rem << 1 ) ^ 0x3000;
            }
            else 
            {
                n_rem = ( n_rem << 1 );
            }
        }
    }
    
    // Final 4-bit remainder is the CRC value
    n_rem = ( ( n_rem >> 12 ) & 0x000F ); 
    
    return ( n_rem ^ 0x00 );
}

void P89BSD012BS::ProcessData()
{
    // Calculate base terms
    // TEMP = A0/3 + A1*2*D2/2^24 + A2*2*(D2/2^24)^2
    m_dT      = (m_D2/POW_2_24);
    m_TEMP    = (m_coeffs[static_cast<uint8_t>(Coefficient::C_A0)]/3) +
                (m_coeffs[static_cast<uint8_t>(Coefficient::C_A1)]*2*m_dT) +
                (m_coeffs[static_cast<uint8_t>(Coefficient::C_A2)]*2*m_dT*m_dT);
    // Q0 = 9
    // Q1 = 11
    // Q2 = 9
    // Q3 = 15
    // Q4 = 15
    // Q5 = 16
    // Q6 = 16
 	
    // Y = (D1 + C0*2^Q0 + C3*2^Q3*D2/2^24 + C4*2^Q4*(D2/2^24)^2) /
    //     (C1*2^Q1 + C5*2^Q5*D2/2^24 + C6*2^Q6*(D2/2^24)^2)
    m_Yn      = m_D1 +       
                (m_coeffs[static_cast<uint8_t>(Coefficient::C_C0)]*POW_2_9) +
                (m_coeffs[static_cast<uint8_t>(Coefficient::C_C3)]*POW_2_15*m_dT) +
                (m_coeffs[static_cast<uint8_t>(Coefficient::C_C4)]*POW_2_15*m_dT*m_dT);
    m_Yd      = (m_coeffs[static_cast<uint8_t>(Coefficient::C_C1)]*POW_2_11) +
                (m_coeffs[static_cast<uint8_t>(Coefficient::C_C5)]*POW_2_16*m_dT) +
                (m_coeffs[static_cast<uint8_t>(Coefficient::C_C6)]*POW_2_16*m_dT*m_dT);
    m_Y       = (m_Yn/m_Yd);
    // P = Y*(1- (C2*2^Q2/2^24)) + C2*2^Q2/2^24*Y^2
    m_Pc      = ((m_coeffs[static_cast<uint8_t>(Coefficient::C_C2)]*POW_2_9)/POW_2_24);
    m_Pi      = (m_Y*(1-m_Pc)) + (m_Y*m_Y*m_Pc);

    m_P       = ((m_Pi-0.1)/0.8)*12;
    // convert bar to psi
    m_P       = m_P * 14.5038;
    // calculate max pressure allowed in psi based on data from SRC
    m_MaxP    = ((m_TEMP - 145.41)/(-5.917)); // max pressure in psi

    // If the measure pressure exceeds the maximum allowable pressure
    if (m_P > m_MaxP) {

    }
	
    // Create data sample with calculated parameters
    m_data.Update(  ( (float)m_TEMP / 1.0f ),     // Temperature
                    ( (float)m_P / 1.0f ),         // Max pressure
                    m_waterMod );
}

// I2C call wrappers
i2c::EI2CResult P89BSD012BS::WriteByte( uint8_t registerIn )
{
    return m_pI2C->WriteByte( m_address, registerIn );
}

i2c::EI2CResult P89BSD012BS::ReadByte( uint8_t *dataOut )
{
    return m_pI2C->ReadByte( m_address, dataOut );
}

i2c::EI2CResult P89BSD012BS::ReadBytes( uint8_t *dataOut, uint8_t numBytesIn )
{
    return m_pI2C->ReadBytes( m_address, dataOut, numBytesIn );
}

i2c::EI2CResult P89BSD012BS::ReadRegisterBytes( uint8_t registerIn, uint8_t *dataOut, uint8_t numBytesIn )
{
    return m_pI2C->ReadRegisterBytes( m_address, registerIn, dataOut, numBytesIn );
}
