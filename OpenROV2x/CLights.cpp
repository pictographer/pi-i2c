#include "SysConfig.h"
#if(HAS_STD_LIGHTS)

// Includes
#include <Arduino.h>
#include <wiringPi.h>
#include "CLights.h"
#include "CMuxes.h"
#include "NCommManager.h"

extern I2C I2C0;
extern CMuxes g_SystemMuxes;
extern uint8_t restart;

namespace
{
	// 1% per 10ms
	const float kPowerDelta = 0.01f;

	bool isGreeting 		= false;
	bool greetState 		= false;
	int greetCycle 			= 0;
	const int cycleCount 	= 6;
    const int greetDelay_ms = 600;

	inline uint32_t PercentToAnalog( float x )
	{	
		if( x < 0.0f )
		{
			return 0;
		}
		else if( x < 0.33f )
		{
			// Linear region, 0-80
			return static_cast<uint32_t>( 242.424f * x );
		}
		else
		{
			// Parabolic region 80-255
			return static_cast<uint32_t>( ( 308.571f * x * x ) - ( 147.426f * x ) + 95.0f );
		}
	}
}

CLights::CLights( uint32_t pinIn, uint32_t pinI2C )
	: m_pin( pinIn, CPin::kDigital, CPin::kOutput ),
          m_pinI2C( pinI2C, CPin::kDigital, CPin::kOutput )
{
}

void CLights::Initialize()
{
   // Reset pin
   m_pin.Reset();
   m_pin.Write( 0 );

   gethostname(m_hostname, HOST_NAME_MAX);

   // Reset timers
   m_controlTimer.Reset();
   m_telemetryTimer.Reset();
}

void CLights::Update( CCommand& commandIn )
{
	// Check for messages
	if( !NCommManager::m_isCommandAvailable )
	{
		return;
	}

	// Handle messages
	if( commandIn.Equals( "lights_tpow" ) )
	{
                // This is actually the KILL command
		// Update the target position
		m_targetPower = orutil::Decode1K( commandIn.m_arguments[1] );

		// TODO: Ideally this unit would have the ability to autonomously set its own target and ack receipt with a separate mechanism
		// Acknowledge target position
		Serial.print( F( "lights_tpow:" ) );
		Serial.print( commandIn.m_arguments[1] );
		Serial.print( ';' );
	        Serial.print( F( "ENDUPDATE:1;" ) );

		// Pass through linearization function
		m_targetPower_an = PercentToAnalog( m_targetPower );

		// Apply ceiling
		if( m_targetPower_an > 255 )
		{
			m_targetPower_an = 255;
		}

		// Directly move to target power
		m_currentPower 		= m_targetPower;
		m_currentPower_an 	= m_targetPower_an;

		// Write the power value to the pin
		// m_pin.Write( m_currentPower_an );
                if (m_targetPower > 0) {
                    // force takeover of the I2C bus
	            m_pinI2C.Write( 1 );
                    // Go through power disarm sequence
                    // SCL_DIO2
                    g_SystemMuxes.WriteExtendedGPIO(RLY_SAF,LOW);
                    // CH1 write 0 - write 1 - wait 1 second - write 0
                    delay(1000);
                    g_SystemMuxes.WriteExtendedGPIO(RLY_SAF,HIGH);
                    delay(1000);
                    g_SystemMuxes.WriteExtendedGPIO(RLY_SAF,LOW);
                    delay(1000);
	            m_pin.Write( 1 );
                }


		// Emit current power
		Serial.print( F( "lights_pow:" ) );
		Serial.print( orutil::Encode1K( m_currentPower ) );
		Serial.print( ';' );
	        Serial.print( F( "ENDUPDATE:1;" ) );
	} else if ( commandIn.Equals( "takeover" ) )
        {
                // OK...we want to maybe force the system to
                // try to takeover
		uint32_t flag = commandIn.m_arguments[1];
		Serial.print( F( "takenover:" ) );
		Serial.print( flag );
		Serial.print( ';' );
	        Serial.print( F( "ENDUPDATE:1;" ) );
                // if we are PI A and get this command then 
                // release the I2C bus
                // If we are PI B and get this command then
                // take over the I2C bus and force a restart
                if (strchr( m_hostname, 'A' ) != NULL) {
                    if (flag == 1) {
                        // release the I2C bus
        	        m_pinI2C.Write( 0 );
                    } else {
                        // force takeover of the I2C bus
	                m_pinI2C.Write( 1 );
                    }
                } else {
                    if (flag == 1) {
                        // force takeover of the I2C bus
        	        m_pinI2C.Write( 1 );
                    } else {
                        // release the I2C bus
	                m_pinI2C.Write( 0 );
                    }
                    // make the module loop restart at the next iteration
                    restart = 1;
                }
        }
#if 0
	else if( commandIn.Equals( "wake" ) )
    {
        // Set greeting state to true and reset timer
        isGreeting = true;
        m_controlTimer.Reset();

		// Set light state to off
		greetState = false;
		m_pin.Write( 0 );
    }

    if( isGreeting )
    {
        if( m_controlTimer.HasElapsed( greetDelay_ms ) )
        {
           // Set to opposite state
		   if( greetState )
		   {
			   greetState = false;
			   m_pin.Write( 0 );
		   }
		   else
		   {
			   greetState = true;
			   m_pin.Write( 150 );
		   }

		   greetCycle++;

		   if( greetCycle >= cycleCount )
		   {
			   // Done blinking
			   isGreeting = false;

			   // Reset pin back to its original value
			   m_pin.Write( m_currentPower_an );
		   }
        }
    }
#endif

}

#endif
