#include "SysConfig.h"
#if( HAS_OROV_CONTROLLERBOARD_25 )

// Includes
#include <Arduino.h>
#include "NDataManager.h"
#include "CControllerBoard.h"
#include <orutil.h>
#include "CPin.h"
#include "CMuxes.h"

extern I2C I2C0;
extern CMuxes g_SystemMuxes;

using namespace ina260;

// File local variables and methods
namespace
{
        // Define the number of samples to keep track of.  The higher the number,
        // the more the readings will be smoothed, but the slower the output will
        // respond to the input.  Using a constant rather than a normal variable lets
        // use this value to determine the size of the readings array.
        const int numReadings = 1;
        int readings[numReadings];      // the readings from the analog input

        orutil::CTimer time;
        orutil::CTimer onesecondtimer;
        orutil::CTimer statustime2;

        int cbindex                     = 0;            // the index of the current reading
        int total                       = 0;            // the running total
        int average                     = 0;                    // the average
        int temppin                     = A8;

        float celsiusTempRead;

        float mapf( long x, long in_min, long in_max, long out_min, long out_max )
        {
                return ( float )( x - in_min ) * ( out_max - out_min ) / ( float )( in_max - in_min ) + out_min;
        }

        void readTemp()
        {
                float volt = mapf( (float)analogRead( temppin ), 0.0f, 1023.0f, 0.0f, 5.0f );
                celsiusTempRead = ( volt - 0.4f ) * 51.28f;
        }

        float readCurrent( int pin )
        {
                return mapf( (float)analogRead( pin ), 0.0f, 1023.0f, 0.0f, 10.0f );
        }

        float readBrdCurrent( int pin )
        {
                return mapf( (float)analogRead( pin ), 0.0f, 1023.0f, 0.0f, 2.0f );
        }

}

CControllerBoard::CControllerBoard()
{
    m_powerSense = NULL;
}

void CControllerBoard::Initialize()
{
        // Reset timers
        time.Reset();
        statustime2.Reset();
        onesecondtimer.Reset();

        m_powerSense = new INA260( &I2C0 );
        m_powerSense->Cmd_SetConfig( 0x6727 );

        // Initialize all the readings to 0:
        for( int thisReading = 0; thisReading < numReadings; ++thisReading )
        {
                readings[ thisReading ] = 0;
        }
}

long CControllerBoard::readLeakDetector()
{
             uint32_t leak =  0;
             leak = g_SystemMuxes.ReadExtendedGPIO(LEAK_SW);
             return(leak);
}

long CControllerBoard::readVcc()
{
             uint32_t volts =  5000;
             g_SystemMuxes.SetPath(SCL_5V_SYS);
             m_powerSense->Cmd_ReadVoltage(&volts);
             return(volts);
}

float CControllerBoard::read20Volts()
{
             float f_volts = 0.0f;
             uint32_t volts =  5000;
             g_SystemMuxes.SetPath(SCL_12V_RPA);
             m_powerSense->Cmd_ReadVoltage(&volts);
             f_volts = (1.0f * volts)/1000.0f;
             return(f_volts);
}

float CControllerBoard::readPiCurrent( uint8_t thePI )
{
             float f_ma = 0.0f;
             uint32_t ma =  5000;
             if (thePI == RPA) {
                g_SystemMuxes.SetPath(SCL_12V_RPA);
             } else {
                g_SystemMuxes.SetPath(SCL_12V_RPB);
             }
             m_powerSense->Cmd_ReadCurrent(&ma);
             f_ma = (1.0f * ma)/1000.0f;
             return(f_ma);
}

void CControllerBoard::Update( CCommand& commandIn )
{
        if( time.HasElapsed( 100 ) )
        {
                // subtract the last reading:
                total = total - readings[cbindex];
                // read from the sensor:
                readings[cbindex] = readBrdCurrent( A0 );

                // add the reading to the total:
                total = total + readings[cbindex];
                // advance to the next position in the array:
                cbindex = cbindex + 1;

                // if we're at the end of the array...
                if( cbindex >= numReadings )
                        // ...wrap around to the beginning:
                {
                        cbindex = 0;
                }

                // calculate the average:
                average = total / numReadings;
        }

        if( onesecondtimer.HasElapsed( 1000 ) )
        {
                readTemp();
                Serial.print( F( "BT1I:" ) );
                Serial.print( readPiCurrent( RPA ) );
                Serial.print( ';' );
                Serial.print( F( "BT2I:" ) );
                Serial.print( readPiCurrent( RPB ) );
                Serial.print( ';' );
                Serial.print( F( "BRDV:" ) );
                Serial.print( read20Volts() );
                Serial.print( ';' );
                Serial.print( F( "AVCC:" ) );
                Serial.print( readVcc() );
                Serial.print( ';' );
                Serial.print( F( "LEAK:" ) );
                Serial.print( readLeakDetector() );
                Serial.print( ';' );
                Serial.print( F( "BRDI:" ) );
                Serial.print( readBrdCurrent( A0 ) );
                Serial.print( ';' );
                Serial.print( F( "SC1I:" ) );
                Serial.print( readCurrent( A3 ) );
                Serial.print( ';' );
                Serial.print( F( "SC2I:" ) );
                Serial.print( readCurrent( A2 ) );
                Serial.print( ';' );
                Serial.print( F( "SC3I:" ) );
                Serial.print( readCurrent( A1 ) );
                Serial.print( ';' );
                Serial.print( F( "BRDT:" ) );
                Serial.print( celsiusTempRead );
                Serial.println( ';' );

        }

        // Update Cape Data voltages and currents
        if( statustime2.HasElapsed( 100 ) )
        {
                NDataManager::m_capeData.VOUT = read20Volts();

                // #315: deprecated: this is the same thing as BRDI:
                NDataManager::m_capeData.IOUT = readBrdCurrent( A0 );

                // Total current draw from batteries:
                NDataManager::m_capeData.BTTI = readPiCurrent( RPA ) + readPiCurrent( RPB );
                NDataManager::m_capeData.FMEM = orutil::FreeMemory();
                NDataManager::m_capeData.UTIM = millis();
        }
}

#endif
