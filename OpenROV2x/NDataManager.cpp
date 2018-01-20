#include "NDataManager.h"

#include "NModuleManager.h"
#include "PinDefinitions.h"

#include <I2C.h>
#if ! defined( ARDUINO_ARCH_AVR )
#include "CSocket.h"
#endif

namespace NDataManager
{
        // Default initialize all data
        TNavData                m_navData;
        TEnvironmentData        m_environmentData;
        TCapeData               m_capeData;
        TThrusterData           m_thrusterData;

        orutil::CTimer          m_timer_1hz;
        orutil::CTimer          m_timer_10hz;

        uint32_t                m_loopsPerSec = 0;

        // Called during Setup() to initialize any DataManager members to specific values
        void Initialize()
        {
                m_thrusterData.MATC             = true;
        }

        void OutputNavData()
        {
                // Print Nav Data on the serial line
                // Serial.print( F( "hdgd:" ) );
                // Serial.print( m_navData.HDGD );
                // Serial.print( ';' );
                // Serial.print( F( "deap:" ) );
                // Serial.print( m_navData.DEEP );
                // Serial.print( ';' );
                // Serial.print( F( "deep:" ) );                // Compatibility for 30.1.x cockpit updates
                // Serial.print( m_navData.DEEP );
                // Serial.print( ';' );
                // Serial.print( F( "pitc:" ) );
                // Serial.print( m_navData.PITC );
                // Serial.print( ';' );
                // Serial.print( F( "roll:" ) );
                // Serial.print( m_navData.ROLL );
                // Serial.print( ';' );
                // Serial.print( F( "yaw:" ) );
                // Serial.print( m_navData.YAW );
                // Serial.print( ';' );
                Serial.print( F( "fthr:" ) );
                Serial.print( m_navData.FTHR );
                Serial.print( ';' );
	        Serial.print( F( "ENDUPDATE:1;" ) );
        }

        void OutputSharedData()
        {
                // Print all other shared data on the serial line

                // Thruster Data
                Serial.print( F( "motorAttached:" ) );
                Serial.print( m_thrusterData.MATC );
                Serial.print( ';' );

                // Cape Data
                Serial.print( F( "fmem:" ) );
                Serial.print( m_capeData.FMEM );
                Serial.print( ';' );
                Serial.print( F( "vout:" ) );
                Serial.print( m_capeData.VOUT );
                Serial.print( ';' );
                Serial.print( F( "iout:" ) );
                Serial.print( m_capeData.IOUT );
                Serial.print( ';' );
                Serial.print( F( "btti:" ) );
                Serial.print( m_capeData.BTTI );
                Serial.print( ';' );
                Serial.print( F( "time:" ) );
                Serial.print( m_capeData.UTIM );
                Serial.print( ';' );

                // Environment Data
                Serial.print( F( "pres:" ) );
                Serial.print( m_environmentData.PRES );
                Serial.print( ';' );
                Serial.print( F( "temp:" ) );
                Serial.print( m_environmentData.TEMP );
                Serial.print( ';' );
	        Serial.print( F( "ENDUPDATE:1;" ) );

                // I2C Debugging
                // -----------------------------------------------------------------
                // RESULT_SUCCESS                   = 0,   // Operation successful
        // RESULT_NACK,                            // Transaction was denied or there was no response

        // // Errors
        // RESULT_ERR_TIMEOUT,                     // Operation timed out
        // RESULT_ERR_FAILED,                      // Operation failed
        // RESULT_ERR_ALREADY_INITIALIZED,         // Interface already initialized
        // RESULT_ERR_INVALID_BAUD,                // Invalid baud rate specified
        // RESULT_ERR_LOST_ARBITRATION,            // Lost arbitration during transaction
        // RESULT_ERR_BAD_ADDRESS,                 // Invalid I2C slave address specified

                Serial.print( F( "i2c.OK:" ) );
                Serial.print( I2C0.GetResultCount( i2c::EI2CResult::RESULT_SUCCESS ) );
                Serial.print( ';' );
                Serial.print( F( "i2c.NACK:" ) );
                Serial.print( I2C0.GetResultCount( i2c::EI2CResult::RESULT_NACK ) );
                Serial.print( ';' );
                Serial.print( F( "i2c.TIMEOUT:" ) );
                Serial.print( I2C0.GetResultCount( i2c::EI2CResult::RESULT_ERR_TIMEOUT ) );
                Serial.print( ';' );
                Serial.print( F( "i2c.FAILED:" ) );
                Serial.print( I2C0.GetResultCount( i2c::EI2CResult::RESULT_ERR_FAILED ) );
                Serial.print( ';' );
                Serial.print( F( "i2c.AL_INIT:" ) );
                Serial.print( I2C0.GetResultCount( i2c::EI2CResult::RESULT_ERR_ALREADY_INITIALIZED ) );
                Serial.print( ';' );
                Serial.print( F( "i2c.BAD_BAUD:" ) );
                Serial.print( I2C0.GetResultCount( i2c::EI2CResult::RESULT_ERR_INVALID_BAUD ) );
                Serial.print( ';' );
                Serial.print( F( "i2c.LOST_ARB:" ) );
                Serial.print( I2C0.GetResultCount( i2c::EI2CResult::RESULT_ERR_LOST_ARBITRATION ) );
                Serial.print( ';' );
                Serial.print( F( "i2c.BAD_ADDR:" ) );
                Serial.print( I2C0.GetResultCount( i2c::EI2CResult::RESULT_ERR_BAD_ADDRESS ) );
                Serial.print( ';' );
	        Serial.print( F( "ENDUPDATE:1;" ) );
        }

        void HandleOutputLoops()
        {
                ++m_loopsPerSec;

                // 1Hz update loop
                if( m_timer_1hz.HasElapsed( 1000 ) )
                {
                        // Send shared data to beaglebone
                        OutputSharedData();

                        // Loops per sec
                        Serial.print( F( "alps:" ) );
                        Serial.print( m_loopsPerSec );
                        Serial.print( ';' );
	                Serial.print( F( "ENDUPDATE:1;" ) );

                        // Reset loop counter
                        m_loopsPerSec = 0;
                }

                // 10Hz Update loop
                if( m_timer_10hz.HasElapsed( 100 ) )
                {
                        // Send nav data to beaglebone
                        OutputNavData();
                }
        }
}
