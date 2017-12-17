#include "SysConfig.h"
#if(HAS_BALLAST)

// Includes
#include <I2C.h>

#include "CBallast.h"
#include "NCommManager.h"
#include "CMuxes.h"

extern CMuxes g_SystemMuxes;
extern I2C I2C0;

// delay to OFF time
#define OFF_TIME( value ) ((4095/255)*value)
// delay to ON time
#define ON_TIME( value )  ((4095/255)*(255-value))

namespace
{
}

CBallast::CBallast()
{
        m_ballast_pwm = new pca9685::PCA9685( &I2C0 );
        m_motor_e = new drv10983::DRV10983( &I2C0 );
        m_valveState = 0;
        m_ballast = 0;
        m_ballast_pre = 0;
}

void CBallast::Initialize()
{
    // enable power to BALLAST
    g_SystemMuxes.WriteExtendedGPIO( BAL_LEDS_EN, LOW );
    // get the valves ready
    // only valve 1 is active now
    g_SystemMuxes.WriteExtendedGPIO( VALVE_1_EN, HIGH );

    // get the PWM ready
    delay(1000);
    g_SystemMuxes.SetPath(SCL_PWM);
    m_ballast_pwm->DigitalWriteLow(pca9685::LED_8);
    m_ballast_pwm->DigitalWriteLow(pca9685::LED_9);
    m_ballast_pwm->UnSleep();

    g_SystemMuxes.SetPath(SCL_ME);
    m_motor_e->Cmd_SetSpeed(0x0000);
    m_motor_e->Cmd_SetConfig(params, sizeof(params));

}

void CBallast::Update( CCommand& commandIn )
{
	// Check for messages
	if( NCommManager::m_isCommandAvailable )
	{
		// Handle messages
		if( commandIn.Equals( "valve" ) )
		{
			// Update the target position

			m_valveState = commandIn.m_arguments[1];
                        if (m_valveState > 0 )
                            g_SystemMuxes.WriteExtendedGPIO( VALVE_1_EN, LOW );
                        else
                            g_SystemMuxes.WriteExtendedGPIO( VALVE_1_EN, HIGH );


		} else 
                if ( commandIn.Equals("ballast") )
                {
                     if( commandIn.m_arguments[1] >= -100 && commandIn.m_arguments[1] <= 100 )
                     {

                        m_ballast = commandIn.m_arguments[1];
                        if ((m_ballast_pre < 0) && (m_ballast > 0)) {
                             g_SystemMuxes.SetPath(SCL_ME);
                             m_motor_e->Cmd_SetSpeed(0x0000);
                          } else {
                             if ((m_ballast < 0) && (m_ballast_pre > 0)) {
                                g_SystemMuxes.SetPath(SCL_ME);
                                m_motor_e->Cmd_SetSpeed(0x0000);
                             }
                          }
                          if (m_ballast == 0) {
                            g_SystemMuxes.WriteExtendedGPIO( VALVE_1_EN, HIGH );
                          } else {
                             g_SystemMuxes.WriteExtendedGPIO( VALVE_1_EN, LOW );
                             if (m_ballast >= 0) {
                                g_SystemMuxes.SetPath(SCL_PWM);
                                m_ballast_pwm->DigitalWriteLow(pca9685::LED_9);
                             } else {
                                g_SystemMuxes.SetPath(SCL_PWM);
                                m_ballast_pwm->DigitalWriteHigh(pca9685::LED_9);
                             }
                          }
                          // -backwards +forwards
                          g_SystemMuxes.SetPath(SCL_ME);
                          m_motor_e->Cmd_SetSpeed(SCALE_SPEED(m_ballast));

                          m_ballast_pre = m_ballast;
                     }

                }
	}
}

#endif
