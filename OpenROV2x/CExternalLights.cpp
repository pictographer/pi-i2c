#include "SysConfig.h"
#if(HAS_EXT_LIGHTS )

// Includes
#include <I2C.h>

#include "CExternalLights.h"
#include "NCommManager.h"
#include "CMuxes.h"

extern CMuxes g_SystemMuxes;
extern I2C I2C0;

// delay to OFF time
#define OFF_TIME( value ) ((4095/255)*value)
// delay to ON time
#define ON_TIME( value )  (0)

namespace
{
	// 1% per 10ms
	const float kPowerDelta = 0.01f;

	inline uint32_t PercentToAnalog( float x )
	{	
              return static_cast<uint32_t> (x*MAX_ALLOWED_POWER);
	}
}

CExternalLights::CExternalLights( uint32_t pinIn )
	: m_pin( pinIn, CPin::kAnalog, CPin::kOutput )
{
        m_led_pwm = new pca9685::PCA9685( &I2C0 );
}

void CExternalLights::Initialize()
{
    // enable power to LEDs
    g_SystemMuxes.WriteExtendedGPIO(BAL_LEDS_EN, LOW);
    // get the PWM ready
    g_SystemMuxes.SetPath(SCL_PWM);
    // top
    m_led_pwm->DigitalWriteLow(pca9685::LED_10);
    // front
    m_led_pwm->DigitalWriteLow(pca9685::LED_11);
    // bottom
    m_led_pwm->DigitalWriteLow(pca9685::LED_12);
    // side
    m_led_pwm->DigitalWriteLow(pca9685::LED_13);
    m_led_pwm->SetPreScale(0x03);
    m_led_pwm->UnSleep();

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
                        // printf("lights: raw %d\n", commandIn.m_arguments[1]);
			// Update the target position
			m_targetPower = orutil::Decode1K( commandIn.m_arguments[1] );
                        // printf("lights: %f\n", m_targetPower);

			// TODO: Ideally this unit would have the ability to autonomously set its own target and ack receipt with a separate mechanism
			// Acknowledge target position
                        delay(50);
			Serial.print( F( "elights_tpow:" ) );
			Serial.print( commandIn.m_arguments[1] );
			Serial.print( ';' );
	                Serial.print( F( "ENDUPDATE:1;" ) );

			// Pass through linearization function
			m_targetPower_an = PercentToAnalog( m_targetPower );

			// Apply ceiling
			if( m_targetPower_an > MAX_ALLOWED_POWER )
			{
				m_targetPower_an = MAX_ALLOWED_POWER;
			}

			// Directly move to target power
			m_currentPower 		= m_targetPower;
			m_currentPower_an 	= m_targetPower_an;

                        g_SystemMuxes.SetPath(SCL_PWM);
                        if (m_currentPower_an == 0) {
                            // top
                            m_led_pwm->DigitalWriteLow(pca9685::LED_10);
                            // front
                            m_led_pwm->DigitalWriteLow(pca9685::LED_11);
                            // bottom
                            m_led_pwm->DigitalWriteLow(pca9685::LED_12);
                            // side
                            m_led_pwm->DigitalWriteLow(pca9685::LED_13);
                        } else {
                            // range 0-255 m_targetPower
                            // for now, all lights are driven together
                            // top
                            m_led_pwm->DigitalWrite(pca9685::LED_10, ON_TIME(m_currentPower_an), OFF_TIME(m_currentPower_an));
                            // front
                            m_led_pwm->DigitalWrite(pca9685::LED_11, ON_TIME(m_currentPower_an), OFF_TIME(m_currentPower_an));
                            // bottom
                            m_led_pwm->DigitalWrite(pca9685::LED_12, ON_TIME(m_currentPower_an), OFF_TIME(m_currentPower_an));
                            // side
                            m_led_pwm->DigitalWrite(pca9685::LED_13, ON_TIME(m_currentPower_an), OFF_TIME(m_currentPower_an));
                        }
                        delay(50);
			// Emit current power
			Serial.print( F( "elights_pow:" ) );
			Serial.print( orutil::Encode1K( m_targetPower ) );
			Serial.print( ';' );
	                Serial.print( F( "ENDUPDATE:1;" ) );
		}
	}
}

#endif
