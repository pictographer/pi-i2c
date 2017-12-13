#include "SysConfig.h"
#if(HAS_EXT_LIGHTS )

// Includes
#include <I2C.h>

#include "CExternalLights.h"
#include "NCommManager.h"
#include "CMuxes.h"

extern CMuxes g_SystemMuxes;
extern I2C I2C0;

namespace
{
	// 1% per 10ms
	const float kPowerDelta = 0.01f;

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

CExternalLights::CExternalLights( uint32_t pinIn )
	: m_pin( pinIn, CPin::kAnalog, CPin::kOutput )
{
        m_monitors = new pca9539::PCA9539( &I2C0 );
        m_led_pwm = new pca9685::PCA9685( &I2C0 );
}

void CExternalLights::Initialize()
{
    // enable power to LEDs
    g_SystemMuxes.SetPath(SCL_CHG);
    m_monitors->PinMode( 0xFF00 );
    m_monitors->DigitalWrite( 0, LOW );
    // get the PWM ready
    // top
    m_led_pwm->DigitalWriteLow(pca9685::LED_10);
    // front
    m_led_pwm->DigitalWriteLow(pca9685::LED_11);
    // bottom
    m_led_pwm->DigitalWriteLow(pca9685::LED_12);
    // side
    m_led_pwm->DigitalWriteLow(pca9685::LED_13);
    m_led_pwm->UnSleep();

    // Reset pin
    m_pin.Reset();
    m_pin.Write( 0 );

    // Reset timers
    m_controlTimer.Reset();
    m_telemetryTimer.Reset();
}

void CExternalLights::Update( CCommand& commandIn )
{
	// Check for messages
	if( NCommManager::m_isCommandAvailable )
	{
		// Handle messages
		if( commandIn.Equals( "elights_tpow" ) )
		{
			// Update the target position
			m_targetPower = orutil::Decode1K( commandIn.m_arguments[1] );

			// TODO: Ideally this unit would have the ability to autonomously set its own target and ack receipt with a separate mechanism
			// Acknowledge target position
			Serial.print( F( "elights_tpow:" ) );
			Serial.print( commandIn.m_arguments[1] );
			Serial.println( ';' );

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
                        g_SystemMuxes.SetPath(SCL_PWM);
                        // range 0-255 m_targetPower

			// Emit current power
			Serial.print( F( "elights_pow:" ) );
			Serial.print( orutil::Encode1K( m_currentPower ) );
			Serial.print( ';' );
		}
	}
}

#endif
