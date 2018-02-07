#include "SysConfig.h"
#if(HAS_BALLAST)

// Includes
#include <I2C.h>

#include "CBallast.h"
#include "NCommManager.h"
#include "CMuxes.h"
#include "CP89BSD012BS.h"

extern CMuxes g_SystemMuxes;
extern I2C I2C0;
extern CP89BSD012BS m_p89bsd012bs;

// delay to OFF time
#define OFF_TIME( value ) ((4095/100)*(std::abs(value)))
// delay to ON time
#define ON_TIME( value )  (0)

namespace
{
}

CBallast::CBallast()
{
#ifdef OLD_BOARD
        m_ballast_pwm = new pca9685::PCA9685( &I2C0, pca9685::PCA9685_ADDRESS_40 );
        m_motor_e = new drv10983::DRV10983( &I2C0 );
#else
        m_ballast_pwm = new pca9685::PCA9685( &I2C0, pca9685::PCA9685_ADDRESS_73 );
#endif
        m_valveState = 0;
        m_ballast = 0;
        m_ballast_pre = 0;
        m_down = 0;
}

void CBallast::Initialize()
{
    // enable power to BALLAST
#ifdef OLD_BOARD
    g_SystemMuxes.WriteExtendedGPIO( BAL_LEDS_EN, LOW );
#else
    g_SystemMuxes.WriteExtendedGPIO( BAL_EN, LOW );
#endif
    // get the valves ready
    // only valve 1 is active now
#ifdef OLD_BOARD
    g_SystemMuxes.WriteExtendedGPIO( VALVE_1_EN, HIGH );
#else
    g_SystemMuxes.WriteExtendedGPIO( VALVE_SER_EN, LOW );
#endif

    // get the PWM ready
    delay(1000);
#ifdef OLD_BOARD
    g_SystemMuxes.SetPath(SCL_PWM);
    m_ballast_pwm->DigitalWriteLow(pca9685::LED_8);
    m_ballast_pwm->DigitalWriteLow(pca9685::LED_9);
    m_ballast_pwm->UnSleep();

    g_SystemMuxes.SetPath(SCL_ME);
    m_motor_e->Cmd_SetSpeed(0x0000);
    m_motor_e->Cmd_SetConfig(params, sizeof(params));
#else
    g_SystemMuxes.SetPath(SCL_NONE);
    m_ballast_pwm->DigitalWriteLow(pca9685::LED_4);
    m_ballast_pwm->DigitalWriteLow(pca9685::LED_9);
    m_ballast_pwm->UnSleep();
#endif

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
#ifdef OLD_BOARD
                        if (m_valveState > 0 )
                            g_SystemMuxes.WriteExtendedGPIO( VALVE_1_EN, LOW );
                        else
                            g_SystemMuxes.WriteExtendedGPIO( VALVE_1_EN, HIGH );
#else
                        if (m_valveState > 0 )
                            g_SystemMuxes.WriteExtendedGPIO( VALVE_SER_EN, LOW );
                        else
                            g_SystemMuxes.WriteExtendedGPIO( VALVE_SER_EN, HIGH );
#endif

		} else 
                if ( commandIn.Equals("ballast") )
                {
                     if( commandIn.m_arguments[1] >= -100 && commandIn.m_arguments[1] <= 100 )
                     {

                        // check if the pressure reading is above the allowable maximum
                        // if it is then only allow reduction of pressure in the reservoir
                        if ((commandIn.m_arguments[1] > 0) && (m_p89bsd012bs.GetMaxPressureFlag())) {
                           Stop();
                           return;
                        }
                        // process the command normally
                        m_ballast = commandIn.m_arguments[1];
                        if ((m_ballast_pre <= 0) && (m_ballast >= 0)) {
#ifdef OLD_BOARD
                             g_SystemMuxes.SetPath(SCL_ME);
                             m_motor_e->Cmd_SetSpeed(0x0000);
#else
                             g_SystemMuxes.SetPath(SCL_NONE);
                             m_ballast_pwm->DigitalWriteLow(pca9685::LED_9);
                             delay(10);
#endif
                          } else {
                             if ((m_ballast <= 0) && (m_ballast_pre >= 0)) {
#ifdef OLD_BOARD
                                g_SystemMuxes.SetPath(SCL_ME);
                                m_motor_e->Cmd_SetSpeed(0x0000);
#else
                                g_SystemMuxes.SetPath(SCL_NONE);
                                m_ballast_pwm->DigitalWriteLow(pca9685::LED_9);
                                delay(10);
#endif
                             }
                          }
                          if (m_ballast == 0) {
#ifdef OLD_BOARD
                            g_SystemMuxes.WriteExtendedGPIO( VALVE_1_EN, HIGH );
#else
                            g_SystemMuxes.WriteExtendedGPIO( VALVE_SER_EN, HIGH );
                            m_down = 0;
#endif
                          } else {
#ifdef OLD_BOARD
                             g_SystemMuxes.WriteExtendedGPIO( VALVE_1_EN, LOW );
#else
                             g_SystemMuxes.WriteExtendedGPIO( VALVE_SER_EN, LOW );
#endif
                             if (m_ballast >= 0) {
#ifdef OLD_BOARD
                                g_SystemMuxes.SetPath(SCL_PWM);
#else
                                g_SystemMuxes.SetPath(SCL_NONE);
#endif
                                if (m_ballast > 0) m_down = 1;
                                m_ballast_pwm->DigitalWriteLow(pca9685::LED_9);
                             } else {
#ifdef OLD_BOARD
                                g_SystemMuxes.SetPath(SCL_PWM);
#else
                                g_SystemMuxes.SetPath(SCL_NONE);
#endif
                                m_down = 0;
                                m_ballast_pwm->DigitalWriteHigh(pca9685::LED_9);
                             }
                          }
                          // -backwards +forwards
#ifdef OLD_BOARD
                          g_SystemMuxes.SetPath(SCL_ME);
                          m_motor_e->Cmd_SetSpeed(SCALE_SPEED(m_ballast));
#else
                          delay(10);
                          g_SystemMuxes.SetPath(SCL_NONE);
                          m_ballast_pwm->DigitalWrite(pca9685::LED_4, ON_TIME(m_ballast), OFF_TIME(m_ballast));
#endif
                          m_ballast_pre = m_ballast;
                     }

                }
	}
}

void CBallast::Stop() {
       if (m_down == 1) {
           g_SystemMuxes.WriteExtendedGPIO( VALVE_SER_EN, HIGH );
           g_SystemMuxes.SetPath(SCL_NONE);
           m_ballast_pwm->DigitalWriteLow(pca9685::LED_4);
           m_ballast_pre = 0;
       }
}

uint8_t CBallast::CheckAndStop() {
       // advance the state machine enough to force a pressure measurement
       m_p89bsd012bs.ForcePressureMeasurement();
       if ((m_p89bsd012bs.GetMaxPressureFlag()) && (m_down == 1)) {
           g_SystemMuxes.WriteExtendedGPIO( VALVE_SER_EN, HIGH );
           g_SystemMuxes.SetPath(SCL_NONE);
           m_ballast_pwm->DigitalWriteLow(pca9685::LED_4);
           m_ballast_pre = 0;
           return( 1 );
       }
       return( 0 );
}

void CBallast::Drive( int32_t value ) {
       m_ballast = value;
       if (CheckAndStop() > 0) {
           return;
       }

       if ((m_ballast_pre <= 0) && (m_ballast >= 0)) {
            g_SystemMuxes.SetPath(SCL_NONE);
            m_ballast_pwm->DigitalWriteLow(pca9685::LED_9);
            delay(10);
         } else {
            if ((m_ballast <= 0) && (m_ballast_pre >= 0)) {
               g_SystemMuxes.SetPath(SCL_NONE);
               m_ballast_pwm->DigitalWriteLow(pca9685::LED_9);
               delay(10);
            }
         }
         if (m_ballast == 0) {
           g_SystemMuxes.WriteExtendedGPIO( VALVE_SER_EN, HIGH );
           m_down = 0;
         } else {
            g_SystemMuxes.WriteExtendedGPIO( VALVE_SER_EN, LOW );
            if (m_ballast >= 0) {
               g_SystemMuxes.SetPath(SCL_NONE);
               if (m_ballast > 0) m_down = 1;
               m_ballast_pwm->DigitalWriteLow(pca9685::LED_9);
            } else {
               g_SystemMuxes.SetPath(SCL_NONE);
               m_down = 0;
               m_ballast_pwm->DigitalWriteHigh(pca9685::LED_9);
            }
         }
         // -backwards +forwards
         delay(10);
         g_SystemMuxes.SetPath(SCL_NONE);
         m_ballast_pwm->DigitalWrite(pca9685::LED_4, ON_TIME(m_ballast), OFF_TIME(m_ballast));
         m_ballast_pre = m_ballast;
}

#endif
