
////////////////////////////////////////////////////////////////////////////
//
//  This file is part of MPU9150Lib
//
//  Copyright (c) 2013 Pansenti, LLC
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy of
//  this software and associated documentation files (the "Software"), to deal in
//  the Software without restriction, including without limitation the rights to use,
//  copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
//  Software, and to permit persons to whom the Software is furnished to do so,
//  subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
//  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
//  PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
//  HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "MPU9150_Calibration.h"

// AVR version

#if defined( ARDUINO_ARCH_AVR )
#include <EEPROM.h>
#endif

void calLibErase( uint8_t device )
{
#if defined( ARDUINO_ARCH_AVR )
        EEPROM.write( CALLIB_START, 0 ); // just destroy the valid byte
#endif
}

void calLibWrite( uint8_t device, CALLIB_DATA* calData )
{
#if defined( ARDUINO_ARCH_AVR )
        uint8_t* ptr = ( uint8_t* )calData;
        uint8_t length = sizeof( CALLIB_DATA );
        int eeprom = CALLIB_START;

        calData->valid = CALLIB_DATA_VALID;

        for( uint8_t i = 0; i < length; i++ )
        {
                EEPROM.write( eeprom + i, *ptr++ );
        }
#endif
}

bool calLibRead( uint8_t device, CALLIB_DATA* calData )
{
#if defined( ARDUINO_ARCH_AVR )
        uint8_t* ptr = ( uint8_t* )calData;
        uint8_t length = sizeof( CALLIB_DATA );
        int eeprom = CALLIB_START;

        calData->magValid = false;
        calData->accelValid = false;

        if( ( EEPROM.read( eeprom ) != CALLIB_DATA_VALID_LOW ) ||
            ( EEPROM.read( eeprom + 1 ) != CALLIB_DATA_VALID_HIGH ) )
        {
                return false;    // invalid data
        }

        for( uint8_t i = 0; i < length; i++ )
        {
                *ptr++ = EEPROM.read( eeprom + i );
        }
#endif
        return true;
}
