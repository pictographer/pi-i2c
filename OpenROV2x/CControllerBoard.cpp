#include "SysConfig.h"
#if( HAS_OROV_CONTROLLERBOARD_25 )

// Includes
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <Arduino.h>
#include "NDataManager.h"
#include "CControllerBoard.h"
#include <orutil.h>
#include "CPin.h"
#include "CMuxes.h"

extern I2C I2C0;
extern CMuxes g_SystemMuxes;

using namespace ina260;
using namespace bq34z100;

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
    m_chargeSense = NULL;
}

void CControllerBoard::Initialize()
{
        // Reset timers
        time.Reset();
        statustime2.Reset();
        onesecondtimer.Reset();

        m_powerSense = new INA260( &I2C0 );
        g_SystemMuxes.SetPath(SCL_5V_SYS);
        m_powerSense->Cmd_SetConfig( 0x6727 );
        g_SystemMuxes.SetPath(SCL_12V_RPA);
        m_powerSense->Cmd_SetConfig( 0x6727 );
        g_SystemMuxes.SetPath(SCL_12V_RPB);
        m_powerSense->Cmd_SetConfig( 0x6727 );

        m_chargeSense = new BQ34Z100( &I2C0 );

        // Initialize all the readings to 0:
        for( int thisReading = 0; thisReading < numReadings; ++thisReading )
        {
                readings[ thisReading ] = 0;
        }

        g_SystemMuxes.SetPath(SCL_BATT);
        //LoadFsFile(m_chargeSense, "/home/pi/pi-i2c/bq.fs");
        LoadFsFile(m_chargeSense, "/home/pi/pi-i2c/df.fs");
}

void CControllerBoard::LoadFsFile( bq34z100::BQ34Z100 *chargeSense, const char *filename ) {
    struct stat st;
    int nSourceFile;
    int nLength;
    int nDataLength;
    char pBuf[32];
    char pData[64];
    int n, m, a;
    char *pEnd = NULL;
    char *pErr;
    char *pFS, *opFS;
    bool bWriteCmd = false;
    unsigned char nRegister;
    unsigned char nAddress;

    // char *pFS: zero terminated buffer with flashstream file
    printf("BQ34Z100 initialization...\n"); 
    if (stat(filename, &st) != 0) {
      printf("Skipping BQ34Z100 initialization\n"); 
      return;
    };

    printf("source file '%s', size = %d\n\r", filename, st.st_size);
    if ((nSourceFile = open(filename, O_RDONLY)) <  0)
    {
        printf("cannot open data classes source file\n\r");
        return;
    }
    opFS = (char *) malloc(st.st_size);
    if (!opFS) return;
    pFS = opFS;
    read(nSourceFile, pFS, st.st_size);
    close(nSourceFile);
    nLength = strlen(pFS);

    // *pHandle: handle to communications adapter

    m = 0;
    for (n = 0; n < nLength; n++)
        if (pFS[n] != ' ') pFS[m++] = pFS[n];
    pEnd = pFS + m;
    pEnd[0] = 0;

    do {
        switch (*pFS)
        {
            case ';':
                break;
            case 'W':
            case 'C':
                bWriteCmd = *pFS == 'W';
                pFS++;
                if ((*pFS) != ':') return;
                pFS++;
                n = 0;
                while ((pEnd - pFS > 2) && (n < sizeof(pData) + 2) &&(*pFS != '\n'))
                {
                    pBuf[0] = *(pFS++);
                    pBuf[1] = *(pFS++);
                    pBuf[2] = 0;
                    m = strtoul(pBuf, &pErr, 16);
                    if (*pErr) return; 
                    if (n == 0) { nAddress = m; chargeSense->setI2CAddress(m); } // this sets the I2C address which changes when in ROM mode
                    if (n == 1) nRegister = m;
                    if (n > 1) pData[n - 2] = m;
                    n++;
                }
                if (n < 3) return;
                nDataLength = n - 2;
                if (bWriteCmd) {
                    // add the register address to the data stream
                    for (m = nDataLength; m != 0; m--) {
                        pData[m] = pData[m-1];
                    }
                    pData[m] = nRegister;
                    nDataLength++;
                    // swap if addressing control register in normal mode
                    for (m = 1; m < nDataLength; m++) {
                       // swap the control bytes
                       chargeSense->WriteByte(pData[0] + (m-1), pData[m]);
                    } 
                } else {
                    char pDataFromGauge[nDataLength];
                    uint8_t data;
                    // TI documents Updating the bq275xx Firmware at Production
                    // SLUA541A indicates that "C" commands should be a checkpoint
                    // to indicate whether to start again
                    for (m = 0; m < nDataLength; m++) {
                        chargeSense->ReadByte(nRegister+m, data);
                        pDataFromGauge[m] = (char) data;
                    }
                    if (memcmp(pData, pDataFromGauge, nDataLength) != 0) {
                        printf("No match: address: %x register: %x nDataLength: %d\n", nAddress, nRegister, nDataLength );
                    }
                }
                break;
           case 'X':
                pFS++;
                if ((*pFS) != ':') return;
                pFS++;
                n = 0;
                while ((pFS != pEnd) && (*pFS != '\n') &&(n <sizeof(pBuf) - 1))
                {
                    pBuf[n++] = *pFS;
                    pFS++; 
                }
                pBuf[n] = 0;
                n = atoi(pBuf);
                delay(n);
                break;
           default: 
                break;
        }
        while ((pFS != pEnd) && (*pFS != '\n')) pFS++; //skip to next line
        if (pFS != pEnd) pFS++;
       } while (pFS != pEnd);

       free(opFS);
       // change back to the default I2C address
       chargeSense->setI2CAddress((BQ34Z100_ADDRESS<<1));
       
       return; 
}


long CControllerBoard::readLeakDetector()
{
             uint32_t leak =  0;
#ifdef OLD_BOARD
             leak = g_SystemMuxes.ReadExtendedGPIO(LEAK_SW1);
#else
             leak = g_SystemMuxes.ReadExtendedGPIO(LEAK_SW2);
#endif
             return(((leak == 0) ? 1 : 0));
}

long CControllerBoard::readCharge() 
{
             // percent remaining charge
             g_SystemMuxes.SetPath(SCL_BATT);
             return m_chargeSense->getRemaining();
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
                Serial.print( F( "BRDT:" ) );
                Serial.print( celsiusTempRead );
                Serial.print( ';' );
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
                Serial.print( F( "CHARGE:" ) );
                Serial.print( readCharge() );
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
                // this must be last
                Serial.print( F( "ENDUPDATE:1;" ) );

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
