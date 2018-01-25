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
#ifdef OLD_BOARD
        m_led_pwm = new pca9685::PCA9685( &I2C0, pca9685::PCA9685_ADDRESS_40 );
#else
        m_led_pwm = new pca9685::PCA9685( &I2C0, pca9685::PCA9685_ADDRESS_73 );
#endif
}

void CExternalLights::Initialize()
{
    // enable power to LEDs
#ifdef OLD_BOARD
    g_SystemMuxes.WriteExtendedGPIO(BAL_LEDS_EN, LOW);
    // get the PWM ready
    g_SystemMuxes.SetPath(SCL_PWM);
#else
    g_SystemMuxes.WriteExtendedGPIO(BAL_EN, LOW);
    g_SystemMuxes.SetPath(SCL_NONE);
#endif
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

#ifdef OLD_BOARD
                        g_SystemMuxes.SetPath(SCL_PWM);
#else
                        g_SystemMuxes.SetPath(SCL_NONE);
#endif
                        if (m_currentPower_an == 0) {
                            // top
                            if ((m_targetLight == 0) || (m_targetLight == 4))
                               m_led_pwm->DigitalWriteLow(pca9685::LED_10);
                            // front
                            if ((m_targetLight == 0) || (m_targetLight == 1))
                               m_led_pwm->DigitalWriteLow(pca9685::LED_11);
                            // bottom
                            if ((m_targetLight == 0) || (m_targetLight == 2))
                               m_led_pwm->DigitalWriteLow(pca9685::LED_12);
                            // side
                            if ((m_targetLight == 0) || (m_targetLight == 3))
                               m_led_pwm->DigitalWriteLow(pca9685::LED_13);
                        } else {
                            // range 0-255 m_targetPower
                            // top
                            if ((m_targetLight == 0) || (m_targetLight == 4))
                               m_led_pwm->DigitalWrite(pca9685::LED_10, ON_TIME(m_currentPower_an), OFF_TIME(m_currentPower_an));
                            // front
                            if ((m_targetLight == 0) || (m_targetLight == 1))
                               m_led_pwm->DigitalWrite(pca9685::LED_11, ON_TIME(m_currentPower_an), OFF_TIME(m_currentPower_an));
                            // bottom
                            if ((m_targetLight == 0) || (m_targetLight == 2))
                               m_led_pwm->DigitalWrite(pca9685::LED_12, ON_TIME(m_currentPower_an), OFF_TIME(m_currentPower_an));
                            // side
                            if ((m_targetLight == 0) || (m_targetLight == 3))
                               m_led_pwm->DigitalWrite(pca9685::LED_13, ON_TIME(m_currentPower_an), OFF_TIME(m_currentPower_an));
                        }
                        delay(50);
			// Emit current power
			Serial.print( F( "elights_pow:" ) );
			Serial.print( orutil::Encode1K( m_targetPower ) );
			Serial.print( ';' );
	                Serial.print( F( "ENDUPDATE:1;" ) );
		} else {
		        if( commandIn.Equals( "elights_select" ) )
		        {
                            // printf("lights: select %d\n", commandIn.m_arguments[1]);
			    // Update the target position
                            // 0: all lights
                            // 1: camera 1 Front
                            // 2: camera 2 Bottom
                            // 3: camera 3 Top
                            // 4: camera 4 Side
			    m_targetLight = commandIn.m_arguments[1];
			    Serial.print( F( "elights_select:" ) );
			    Serial.print( m_targetLight );
			    Serial.print( ';' );
	                    Serial.print( F( "ENDUPDATE:1;" ) );
		        }
                }
	}
}

#endif
