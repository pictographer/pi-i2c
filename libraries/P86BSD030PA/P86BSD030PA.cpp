// Includes
#include "P86BSD030PA.h"
#include <assert.h>

using namespace p86bsd030pa;

P86BSD030PA::P86BSD030PA( I2C *i2cInterfaceIn )
    : m_pI2C( i2cInterfaceIn )
    , m_address( I2C_ADDRESS )
{
}

// --------------------------------------------------------------
// Public API
// --------------------------------------------------------------

void P86BSD030PA::Tick()
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
                DelayedTransition( EState::CONVERTING_PRESSURE, m_conversionTime_ms );
                m_results.AddResult( EResult::RESULT_ERR_FAILED_SEQUENCE );
            }
            else
            {
                // Success, next read the value and start converting pressure
                DelayedTransition( EState::CONVERTING_TEMPERATURE, m_conversionTime_ms );
            }

            break;
        };

        case EState::CONVERTING_TEMPERATURE:
        {
            // Read pressure
            if( Cmd_ReadPressure() )
            {
                // Failure, start new conversion sequence
                DelayedTransition( EState::CONVERTING_PRESSURE, m_conversionTime_ms );
                m_results.AddResult( EResult::RESULT_ERR_FAILED_SEQUENCE );
            }
            else
            {
                // Start temp conversion
                if( Cmd_StartTempConversion() )
                {
                    // Failure, start new conversion sequence
                    DelayedTransition( EState::CONVERTING_PRESSURE, m_conversionTime_ms );
                    m_results.AddResult( EResult::RESULT_ERR_FAILED_SEQUENCE );
                }
                else
                {
                    // Success
                    DelayedTransition( EState::PROCESSING_DATA, m_conversionTime_ms );
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

void P86BSD030PA::HardReset()
{
    // Perform delayed transition to uninitialized
    DelayedTransition( EState::UNINITIALIZED, kRetryDelay_ms );

    // Increment hard reset counter
    m_results.AddResult( EResult::RESULT_ERR_HARD_RESET );

    // Clear the sequence error counter
    ClearResultCount( EResult::RESULT_ERR_FAILED_SEQUENCE );
}

void P86BSD030PA::FullReset()
{
    // Perform delayed transition to uninitialized
    DelayedTransition( EState::UNINITIALIZED, kRetryDelay_ms );

    // Clear all results
    m_results.Clear();

    // Re-enable device
    m_enabled = true;
}

uint32_t P86BSD030PA::GetResultCount( EResult resultTypeIn )
{
    return m_results.GetResultCount( resultTypeIn );
}

void P86BSD030PA::ClearResultCount( EResult resultTypeIn )
{
    m_results.ClearResult( resultTypeIn );
}

void P86BSD030PA::Disable()
{
    // Disable device entirely
    Transition( EState::DISABLED );

    m_enabled = false;
}

bool P86BSD030PA::IsEnabled()
{
    return m_enabled;
}

bool P86BSD030PA::GetLock()
{
    return m_pI2C->LockAddress( m_address );
}

void P86BSD030PA::FreeLock()
{
    m_pI2C->FreeAddress( m_address );
}

EResult P86BSD030PA::SetOversampleRate( EOversampleRate rateIn )
{
    // this device does not have a settable oversampling rate

    return EResult::RESULT_SUCCESS;
}

EResult P86BSD030PA::SetWaterType( EWaterType typeIn )
{
    // Update the water modifier. No need to adjust state machine.
    m_waterMod = ( ( typeIn == EWaterType::FRESH ) ? kWaterModFresh : kWaterModSalt );

    return EResult::RESULT_SUCCESS;
}

uint32_t P86BSD030PA::GetUpdatePeriod()
{
    // Two conversion periods
    return 2 * m_conversionTime_ms;
}

// --------------------------------------------------------------
// Private Methods
// --------------------------------------------------------------

void P86BSD030PA::Transition( EState stateIn )
{
    // Set new state
    m_state = stateIn;
}

void P86BSD030PA::DelayedTransition( EState nextState, uint32_t millisIn )
{
    // Set delay info
    m_state                 = EState::DELAY;
    m_delay.nextState       = nextState;
    m_delay.delayTime_ms    = millisIn;
    m_delay.timer.Reset(); 
}

EResult P86BSD030PA::Cmd_Reset()
{
    // This device has no reset command
    return EResult::RESULT_SUCCESS;
}

EResult P86BSD030PA::Cmd_ReadCalibrationData()
{
   // this device has no stored calibration data that needs to be loaded
   return EResult::RESULT_SUCCESS;
}

EResult P86BSD030PA::Cmd_StartPresConversion()
{
    uint8_t dummy = 0;

    // reading the device initiates a measurement
    if( ReadByte( &dummy ) )
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

EResult P86BSD030PA::Cmd_StartTempConversion()
{
    uint8_t dummy = 0;

    // reading the device initiates a measurement
    if( ReadByte( &dummy ) )
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

EResult P86BSD030PA::Cmd_ReadPressure()
{
    uint8_t bytes[ 2 ];

    // two bytes of pressure
    // the upper two bits in the first byte are a status
    // the remaining 14 bits are the pressure
    i2c::EI2CResult ret = ReadBytes( bytes, 2 );
    if( ret )
    {
        // I2C Failure
        return (EResult)m_results.AddResult( EResult::RESULT_ERR_I2C_TRANSACTION );
    }

    // slice off the status bits
    // 00 : good data
    // 01 : reserved
    // 10 : stale data
    // 11 : fault
    uint8_t status = bytes[0]&0xC0;
    // Combine the bytes into one integer for the final result
    m_D1 = ( ( uint32_t )(bytes[ 0 ]&0x3F) << 8 ) + ( ( uint32_t )bytes[ 1 ] );

    return EResult::RESULT_SUCCESS;
}

EResult P86BSD030PA::Cmd_ReadTemperature()
{
    uint8_t bytes[ 4 ];

    // two bytes of pressure
    // the upper two bits in the first byte are a status
    // the remaining 14 bits are the pressure
    // this is followed by two bytes of temperature
    // the first byte is the upper 8 bits of temperature
    // the upper 3 bits of the second byte are the lower 3 bits
    // for a total of 11 bits of temperature
    i2c::EI2CResult ret = ReadBytes( bytes, 4 );

    if( ret )
    {
        // I2C Failure
        return (EResult)m_results.AddResult( EResult::RESULT_ERR_I2C_TRANSACTION );
    }

    // slice off the status bits
    // 00 : good data
    // 01 : reserved
    // 10 : stale data
    // 11 : fault
    uint8_t status = bytes[0]&0xC0;
    // Combine the bytes into one integer for the final result
    m_D2 = ( ( uint32_t )bytes[ 2 ] << 8 ) + ( ( uint32_t )(bytes[ 3 ]&0xE0) ) ;
    // right align it
    m_D2 >>= 5;

    return EResult::RESULT_SUCCESS;
}

uint8_t P86BSD030PA::CalculateCRC4()
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

void P86BSD030PA::ProcessData()
{
    m_TEMP2 = ((int32_t) ((m_D2*200)/2047)) - 50; // degrees C
    m_P = (((m_D1 - (16383/10))*(27-3))/((8*16383)/10)) + 3; // psi
    m_P *= 68.9476; // psi to mbar
	
    // Create data sample with calculated parameters
    m_data.Update(  ( (float)m_TEMP2 / 100.0f ),    // Temperature
                    ( (float)m_P / 10.0f ),         // Pressure
                    m_waterMod );
}

// I2C call wrappers
i2c::EI2CResult P86BSD030PA::WriteByte( uint8_t registerIn )
{
    return m_pI2C->WriteByte( m_address, registerIn );
}

i2c::EI2CResult P86BSD030PA::ReadByte( uint8_t *dataOut )
{
    return m_pI2C->ReadByte( m_address, dataOut );
}

i2c::EI2CResult P86BSD030PA::ReadBytes( uint8_t *dataOut, uint8_t numBytesIn )
{
    return m_pI2C->ReadBytes( m_address, dataOut, numBytesIn );
}
