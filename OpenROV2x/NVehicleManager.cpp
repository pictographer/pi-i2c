// Includes
#include <I2C.h>
#include "NVehicleManager.h"
#include "NCommManager.h"
#include "CCommand.h"
#include "CompileOptions.h"
#include "CMuxes.h"

#if !defined( ARDUINO_ARCH_AVR )
#include "CSocket.h"
#endif

extern CMuxes g_SystemMuxes;
extern I2C I2C0;

namespace NVehicleManager
{
        // ---------------------------------------------------------
        // Variable initialization
        // ---------------------------------------------------------
        pca9539::PCA9539  *m_power;

        // TODO: Move
        uint32_t m_throttleSmoothingIncrement   = 40;
        uint32_t m_deadZoneMin                                  = 50;
        uint32_t m_deadZoneMax                                  = 50;

        // ---------------------------------------------------------
        // Method Definitions
        // ---------------------------------------------------------

        void Initialize()
        {
            m_power = new pca9539::PCA9539( &I2C0 );
            // Power System Arm
            // '9539 low -> high (1 second) -> low
            g_SystemMuxes.SetPath(SCL_DIO2);
            m_power->PinMode(0xFF00);
            m_power->DigitalWrite(1, 0x00);
            delay(1000);
            m_power->DigitalWrite(1, 0x01);
            delay(1000);
            m_power->DigitalWrite(1, 0x00);

        }

        void HandleMessages( CCommand &commandIn )
        {
                if( NCommManager::m_isCommandAvailable )
                {
                        if( commandIn.Equals( "version" ) )
                        {
                                // SHA1 hash of all of the source used to build the firmware
                                // This gets automatically generated at compile time by the ArduinoBuilder library and appended to CompileOptions.h
                                Serial.println( VERSION_HASH );
                        }
                        else if( commandIn.Equals( "wake" ) )
                        {
                                // Acknowledge greeting
                                Serial.println( "awake:;" );
                        }
                        // TODO: These should be handled by the control module
                        else if( commandIn.Equals( "reportSetting" ) )
                        {
                                Serial.print( F( "*settings:" ) );
                                Serial.print( F( "smoothingIncrement|" ) );             Serial.print( m_throttleSmoothingIncrement );   Serial.print( ',' );
                                Serial.print( F( "deadZone_min|" ) );                   Serial.print( m_deadZoneMin );                                  Serial.print( ',' );
                                Serial.print( F( "deadZone_max|" ) );                   Serial.print( m_deadZoneMax );                                  Serial.println( ';' );
                        }
                        else if( commandIn.Equals( "updateSetting" ) )
                        {
                                // TODO: Need to update the motors with new deadZone setting. Probably move
                                // deadzone to the thruster resposibilitiy
                                m_throttleSmoothingIncrement    = commandIn.m_arguments[1];
                                m_deadZoneMin                                   = commandIn.m_arguments[2];
                                m_deadZoneMax                                   = commandIn.m_arguments[3];
                        }
                }
        }
}
