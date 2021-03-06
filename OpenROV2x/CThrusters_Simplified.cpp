#include "SysConfig.h"
#if( THRUSTER_CONFIGURATION == THRUSTER_CONFIG_SIMPLIFIED )

// Includes
#include <cmath>
#include <I2C.h>

#include "CThrusters.h"
#include "NVehicleManager.h"
#include "NDataManager.h"
#include "CMotor.h"
#include "CMuxes.h"
#include <orutil.h>
#include "CPin.h"
#include "DRV10983.h"
#include "PCA9685.h"
#include "CBouyancy.h"
#include "CP86BSD030PA.h"

// delay to OFF time
#define OFF_TIME( value ) ((4095/100)*(std::abs(value)))
// delay to ON time
#define ON_TIME( value )  (0)

extern CMuxes g_SystemMuxes;
extern I2C I2C0;
extern CBouyancy m_bouyancy;
extern CP86BSD030PA m_p86bsd030pa;

template<class T>
const T& constrain(const T& x, const T& a, const T& b) {
    if(x < a) {
        return a;
    }
    else if(b < x) {
        return b;
    }
    else
        return x;
}

// Static variable initialization
const int CThrusters::kMotorCount = 4;

namespace
{
    // source: nsr1215_2.csv
    uint8_t params[12] = {
	0xEA,
	0xC9,
	0x3A,
	0x04,
	0xC0,
	0xFB,
	0x7E,
	0xB0,
	0x0F,
	0x88,
	0x0E,
	0x0C
   };

    // motor controller
    // A - port horizontal
    // New: A - port vertical
    drv10983::DRV10983 *motor_a;
    // 0x72 CH0
    // B - starboard horizontal
    // New: B - starboard vertical
    drv10983::DRV10983 *motor_b;
    // 0x72 CH1
    // C - port vertical
    // New: C - port horizontal
    drv10983::DRV10983 *motor_c;
    // 0x72 CH2
    // D - starboard vertical
    // New: D - starboard horizontal
    drv10983::DRV10983 *motor_d;
    // 0x72 CH3
    // motor enable signals
    // AB enable
    // 0x72 CH7 then 0x74 PO0 low
    // CD enable
    // 0x72 CH7 then 0x74 PO3 low
    // motor direction signals
    pca9685::PCA9685 *motor_signals;
    // A forward/backward (PWM high or low)
    // 0x72 CH6 then 0x40 LED1 low (forward) high (reverse)
    // B forward/backward (PWM high or low)
    // 0x72 CH6 then 0x40 LED3 low (forward) high (reverse)
    // C forward/backward (PWM high or low)
    // 0x72 CH6 then 0x40 LED5 low (forward) high (reverse)
    // D forward/backward (PWM high or low)
    // 0x72 CH6 then 0x40 LED7 low (forward) high (reverse)

    CMotor port_motor( PIN_PORT_MOTOR );
    CMotor port_vertical_motor( PIN_PORT_VERTICAL_MOTOR );
    CMotor starboard_motor( PIN_STARBOARD_MOTOR );
    CMotor starboard_vertical_motor( PIN_STAR_VERTICAL_MOTOR );

    int new_p	= MOTOR_TARGET_NEUTRAL_US;
    int new_s	= MOTOR_TARGET_NEUTRAL_US;
    int new_vp = MOTOR_TARGET_NEUTRAL_US;
    int new_vs = MOTOR_TARGET_NEUTRAL_US;
    int new_st = 0; //strafe differential
    int p = MOTOR_TARGET_NEUTRAL_US;
    int vp = MOTOR_TARGET_NEUTRAL_US;
    int vs = MOTOR_TARGET_NEUTRAL_US;
    int vp2 = MOTOR_TARGET_NEUTRAL_US;
    int vs2 = MOTOR_TARGET_NEUTRAL_US;
    int s = MOTOR_TARGET_NEUTRAL_US;
    int st;

    float trg_throttle,trg_yaw,trg_lift,trg_strafe;
    float p_trg_throttle = 0.0, p_trg_yaw = 0.0, p_trg_lift = 0.0, p_trg_strafe = 0.0;
    int trg_motor_power;
    int maxVtransDelta;


    orutil::CTimer controltime;
    orutil::CTimer thrusterOutput;
    bool bypasssmoothing;

    bool isGreeting = false;
    const int greetDelay_ms = 100;

#ifdef PIN_ENABLE_ESC
    bool canPowerESCs = true;
    CPin escpower( PIN_ENABLE_ESC, CPin::kDigital, CPin::kOutput );
#else
    bool canPowerESCs = false;
#endif
}

void CThrusters::Initialize()
{
    // leave them on for now at time zero
    m_enable = 1;
    m_lastTargetDepth = 0.3;
    m_first_zero = 0;
    m_lift_zero_time = (uint32_t) -1;

#ifdef OLD_BOARD
    motor_a = new drv10983::DRV10983( &I2C0 );
    motor_b = new drv10983::DRV10983( &I2C0 );
    motor_c = new drv10983::DRV10983( &I2C0 );
    motor_d = new drv10983::DRV10983( &I2C0 );
    motor_signals = new pca9685::PCA9685( &I2C0, pca9685::PCA9685_ADDRESS_40 );
    // setup the IO expander inputs and output directions
    // and power the motor controllers
    g_SystemMuxes.WriteExtendedGPIO(MAB_EN,LOW);
    g_SystemMuxes.WriteExtendedGPIO(MCD_EN,LOW);
    delay(1000);
    // enable the motors
    g_SystemMuxes.SetPath(SCL_MA);
    motor_a->Cmd_SetSpeed(0x0000);
    motor_a->Cmd_SetConfig(params, sizeof(params));
    // intialize the motor parameters
    g_SystemMuxes.SetPath(SCL_MB);
    motor_b->Cmd_SetSpeed(0x0000);
    motor_b->Cmd_SetConfig(params, sizeof(params));
    g_SystemMuxes.SetPath(SCL_MC);
    motor_c->Cmd_SetSpeed(0x0000);
    motor_c->Cmd_SetConfig(params, sizeof(params));
    g_SystemMuxes.SetPath(SCL_MD);
    motor_d->Cmd_SetSpeed(0x0000);
    motor_d->Cmd_SetConfig(params, sizeof(params));
    g_SystemMuxes.SetPath(SCL_PWM);
    // get the PWM alive
    motor_signals->UnSleep();
    // initialize all directions LOW (forward)
    motor_signals->DigitalWriteLow(pca9685::LED_1);
    motor_signals->DigitalWriteLow(pca9685::LED_3);
    motor_signals->DigitalWriteLow(pca9685::LED_5);
    motor_signals->DigitalWriteLow(pca9685::LED_7);
#else
    motor_signals = new pca9685::PCA9685( &I2C0, pca9685::PCA9685_ADDRESS_73 );
    // setup the IO expander inputs and output directions
    // and power the motor controllers
    g_SystemMuxes.WriteExtendedGPIO(MCTRL_RESET,HIGH);
    g_SystemMuxes.WriteExtendedGPIO(PORT_EN,LOW);
    g_SystemMuxes.WriteExtendedGPIO(STAR_EN,LOW);
    g_SystemMuxes.SetPath(SCL_NONE);
    // get the PWM alive
    motor_signals->UnSleep();
    delay(1000);
    // initialize all directions LOW (forward)
    // motors
    motor_signals->DigitalWriteLow(pca9685::LED_0);
    motor_signals->DigitalWriteLow(pca9685::LED_1);
    motor_signals->DigitalWriteLow(pca9685::LED_2);
    motor_signals->DigitalWriteLow(pca9685::LED_3);
    // directions
    motor_signals->DigitalWriteLow(pca9685::LED_5);
    motor_signals->DigitalWriteLow(pca9685::LED_6);
    motor_signals->DigitalWriteLow(pca9685::LED_7);
    motor_signals->DigitalWriteLow(pca9685::LED_8);
#endif

    port_motor.m_negativeDeadzoneBuffer = NVehicleManager::m_deadZoneMin;
    port_motor.m_positiveDeadzoneBuffer = NVehicleManager::m_deadZoneMax;
    port_motor.Activate();

    port_vertical_motor.m_negativeDeadzoneBuffer = NVehicleManager::m_deadZoneMin;
    port_vertical_motor.m_positiveDeadzoneBuffer = NVehicleManager::m_deadZoneMax;
    port_vertical_motor.Activate();

    starboard_vertical_motor.m_negativeDeadzoneBuffer = NVehicleManager::m_deadZoneMin;
    starboard_vertical_motor.m_positiveDeadzoneBuffer = NVehicleManager::m_deadZoneMax;
    starboard_vertical_motor.Activate();

    starboard_motor.m_negativeDeadzoneBuffer = NVehicleManager::m_deadZoneMin;
    starboard_motor.m_positiveDeadzoneBuffer = NVehicleManager::m_deadZoneMax;
    starboard_motor.Activate();

    thrusterOutput.Reset();
    controltime.Reset();

    bypasssmoothing = false;

    #ifdef PIN_ENABLE_ESC
    escpower.Reset();
    escpower.Write( 1 ); //Turn on the ESCs
    #endif
}

void CThrusters::Update( CCommand& command )
{
    //
    // check if thruster commands are enabled
    // skip this whole thing if thrusters are off
    //
    if (m_enable == 0) return;
    // check if the lift is zero and how long it has been zero
    // how long the lift has been zero
    uint32_t now = millis();
    if (m_bouyancy.BouyancyActive() && (p_trg_lift == 0) && (m_lift_zero_time != (uint32_t) -1) && ((now - m_lift_zero_time) > 500)) {
       float newTargetDepth = m_p86bsd030pa.GetDepth();
       // we've been at zero for a while, let's see if we have dropped
       if (std::abs(newTargetDepth - m_lastTargetDepth) > 0.01) {
           // get the system stable at this
           // depth
           m_bouyancy.SetTargetDepth( newTargetDepth );
           // make sure the PIDs are running
           m_bouyancy.RunPID( 1, 1 );
	   m_lastTargetDepth = newTargetDepth;
       }
    }

    if( command.Equals( "mtrmod1" ) )
    {
#if 0
        port_motor.m_positiveModifier = command.m_arguments[1] / 100;
        port_vertical_motor.m_positiveModifier = command.m_arguments[2] / 100;
        starboard_vertical_motor.m_positiveModifier = command.m_arguments[2] / 100;
        starboard_motor.m_positiveModifier = command.m_arguments[3] / 100;
#endif
    }

    else if( command.Equals( "mtrmod2" ) )
    {
#if 0
        port_motor.m_negativeModifier = command.m_arguments[1] / 100;
        port_vertical_motor.m_negativeModifier = command.m_arguments[2] / 100;
        starboard_vertical_motor.m_negativeModifier = command.m_arguments[2] / 100;
        starboard_motor.m_negativeModifier = command.m_arguments[3] / 100;
#endif
    }

    else if( command.Equals( "rmtrmod" ) )
    {
#if 0
        Serial.print( F( "mtrmod:" ) );
        Serial.print( port_motor.m_positiveModifier );
        Serial.print( "," );
        Serial.print( port_vertical_motor.m_positiveModifier );
        Serial.print( "," );
        Serial.print( starboard_vertical_motor.m_positiveModifier );
        Serial.print( "," );
        Serial.print( starboard_motor.m_positiveModifier );
        Serial.print( "," );
        Serial.print( port_motor.m_negativeModifier );
        Serial.print( "," );
        Serial.print( port_vertical_motor.m_negativeModifier );
        Serial.print( "," );
        Serial.print( starboard_vertical_motor.m_negativeModifier );
        Serial.print( "," );
        Serial.print( starboard_motor.m_negativeModifier );
        Serial.print( ";" );
	Serial.print( F( "ENDUPDATE:1;" ) );
#endif
    }

    else if( command.Equals( "port" ) )
    {
#if 0
        //ignore corrupt data
        if( command.m_arguments[1] > 999 && command.m_arguments[1] < 2001 )
        {
            p = command.m_arguments[1];

            if( command.m_arguments[2] == 1 )
            {
                bypasssmoothing = true;
            }
        }
#endif
    }

    else if( command.Equals( "vertical" ) )
    {
#if 0
        //ignore corrupt data
        if( command.m_arguments[1] > 999 && command.m_arguments[1] < 2001 )
        {
            vp = command.m_arguments[1];
            vs = vp;

            if( command.m_arguments[2] == 1 )
            {
                bypasssmoothing = true;
            }
        }
#endif
    }

    else if( command.Equals( "starboard" ) )
    {
#if 0
        //ignore corrupt data
        if( command.m_arguments[1] > 999 && command.m_arguments[1] < 2001 )
        {
            s = command.m_arguments[1];

            if( command.m_arguments[2] == 1 )
            {
                bypasssmoothing = true;
            }
        }
#endif
    }

    else if( command.Equals( "thro" ) || command.Equals( "yaw" ) )
    {
        if( command.Equals( "thro" ) )
        {
            if( command.m_arguments[1] >= -100 && command.m_arguments[1] <= 100 )
            {
                trg_throttle = command.m_arguments[1] / 1.0;
                // adjust throttle here
                // direction change
                if ((p_trg_throttle <= 0.0) && (trg_throttle >= 0.0)) {
#ifdef OLD_BOARD
                   g_SystemMuxes.SetPath(SCL_MA);
                   motor_a->Cmd_SetSpeed(0x0000);
                   g_SystemMuxes.SetPath(SCL_MB);
                   motor_b->Cmd_SetSpeed(0x0000);
#else
                   // Now Motors C and D
                   g_SystemMuxes.SetPath(SCL_NONE);
                   motor_signals->DigitalWriteLow(pca9685::LED_2);
                   delay(10);
#endif
                } else {
                   if ((trg_throttle <= 0.0) && (p_trg_throttle >= 0.0)) {
#ifdef OLD_BOARD
                      g_SystemMuxes.SetPath(SCL_MA);
                      motor_a->Cmd_SetSpeed(0x0000);
                      g_SystemMuxes.SetPath(SCL_MB);
                      motor_b->Cmd_SetSpeed(0x0000);
#else
                      // Now Motors C and D
                      g_SystemMuxes.SetPath(SCL_NONE);
                      motor_signals->DigitalWriteLow(pca9685::LED_2);
                      delay(10);
#endif
                   }
                }
                if (trg_throttle < 0.0) {
#ifdef OLD_BOARD
                   g_SystemMuxes.SetPath(SCL_PWM);
                   motor_signals->DigitalWriteLow(pca9685::LED_1);
                   motor_signals->DigitalWriteHigh(pca9685::LED_3);
#else
                   // Now Motors C and D
                   g_SystemMuxes.SetPath(SCL_NONE);
                   motor_signals->DigitalWriteHigh(pca9685::LED_7);
#endif
                } else {
#ifdef OLD_BOARD
                   g_SystemMuxes.SetPath(SCL_PWM);
                   motor_signals->DigitalWriteHigh(pca9685::LED_1);
                   motor_signals->DigitalWriteLow(pca9685::LED_3);
#else
                   // Now Motors C and D
                   g_SystemMuxes.SetPath(SCL_NONE);
                   motor_signals->DigitalWriteLow(pca9685::LED_7);
#endif
                }
                // -backwards +forwards
#ifdef OLD_BOARD
                g_SystemMuxes.SetPath(SCL_MA);
                motor_a->Cmd_SetSpeed(SCALE_SPEED(trg_throttle));
                g_SystemMuxes.SetPath(SCL_MB);
                motor_b->Cmd_SetSpeed(SCALE_SPEED(trg_throttle));
#else
                // Now Motors C and D
                delay(10);
                g_SystemMuxes.SetPath(SCL_NONE);
                motor_signals->DigitalWrite(pca9685::LED_2, ON_TIME(trg_throttle), OFF_TIME(trg_throttle));
#endif
                // both vertical motors drive in the same direction
                p_trg_throttle = trg_throttle;
            }
        }

        else if( command.Equals( "yaw" ) )
        {
            //ignore corrupt data
            if( command.m_arguments[1] >= -100 && command.m_arguments[1] <= 100 ) //percent of max turn
            {
                trg_yaw = command.m_arguments[1] / 1.0;
                if ((p_trg_yaw <= 0.0) && (trg_yaw >= 0.0)) {
                   // direction change
#ifdef OLD_BOARD
                   g_SystemMuxes.SetPath(SCL_MA);
                   motor_a->Cmd_SetSpeed(0x0000);
                   g_SystemMuxes.SetPath(SCL_MB);
                   motor_b->Cmd_SetSpeed(0x0000);
#else
                   // Now Motors C and D
                   g_SystemMuxes.SetPath(SCL_NONE);
                   motor_signals->DigitalWriteLow(pca9685::LED_3);
                   delay(10);
#endif
                } else {
                   if ((trg_yaw <= 0.0) && (p_trg_yaw >= 0.0)) {
#ifdef OLD_BOARD
                      g_SystemMuxes.SetPath(SCL_MA);
                      motor_a->Cmd_SetSpeed(0x0000);
                      g_SystemMuxes.SetPath(SCL_MB);
                      motor_b->Cmd_SetSpeed(0x0000);
#else
                      // Now Motors C and D
                      g_SystemMuxes.SetPath(SCL_NONE);
                      motor_signals->DigitalWriteLow(pca9685::LED_3);
                      delay(10);
#endif
                   }
                }
                // adjust yaw here
                // -left +right
                if (trg_yaw >= 0.0) {
#ifdef OLD_BOARD
                   g_SystemMuxes.SetPath(SCL_PWM);
                   motor_signals->DigitalWriteHigh(pca9685::LED_1);
                   motor_signals->DigitalWriteHigh(pca9685::LED_3);
                   g_SystemMuxes.SetPath(SCL_MA);
                   motor_a->Cmd_SetSpeed(SCALE_SPEED(trg_yaw));
                   g_SystemMuxes.SetPath(SCL_MB);
                   motor_b->Cmd_SetSpeed(SCALE_SPEED(trg_yaw));
#else
                   // Now Motors C and D
                   g_SystemMuxes.SetPath(SCL_NONE);
                   motor_signals->DigitalWriteLow(pca9685::LED_8);
                   delay(10);
                   motor_signals->DigitalWrite(pca9685::LED_3, ON_TIME(trg_yaw), OFF_TIME(trg_yaw));
#endif
                } else {
#ifdef OLD_BOARD
                   g_SystemMuxes.SetPath(SCL_PWM);
                   motor_signals->DigitalWriteLow(pca9685::LED_1);
                   motor_signals->DigitalWriteLow(pca9685::LED_3);
                   g_SystemMuxes.SetPath(SCL_MA);
                   motor_a->Cmd_SetSpeed(SCALE_SPEED(trg_yaw));
                   g_SystemMuxes.SetPath(SCL_MB);
                   motor_b->Cmd_SetSpeed(SCALE_SPEED(trg_yaw));
#else
                   // Now Motors C and D
                   g_SystemMuxes.SetPath(SCL_NONE);
                   motor_signals->DigitalWriteHigh(pca9685::LED_8);
                   delay(10);
                   motor_signals->DigitalWrite(pca9685::LED_3, ON_TIME(trg_yaw), OFF_TIME(trg_yaw));
#endif
                }
                p_trg_yaw = trg_yaw;
            }
        }


        // The code below spreads the throttle spectrum over the possible range
        // of the motor. Not sure this belongs here or should be placed with
        // deadzon calculation in the motor code.
        if( trg_throttle >= 0 )
        {
            p = 1500 + ( 500.0 / std::abs( port_motor.m_positiveModifier ) ) * trg_throttle;
            s = p;
        }
        else
        {
            p = 1500 + ( 500.0 / std::abs( port_motor.m_negativeModifier ) ) * trg_throttle;
            s = p;
        }

        trg_motor_power = s;

        int turn = trg_yaw * 250; //max range due to reverse range

        if( trg_throttle >= 0 )
        {
            int offset = ( std::abs( turn ) + trg_motor_power ) - 2000;

            if( offset < 0 )
            {
                offset = 0;
            }

            p = trg_motor_power + turn - offset;
            s = trg_motor_power - turn - offset;
        }
        else
        {
            int offset = 1000 - ( trg_motor_power - std::abs( turn ) );

            if( offset < 0 )
            {
                offset = 0;
            }

            p = trg_motor_power + turn + offset;
            s = trg_motor_power - turn + offset;
        }

    }
#if 0
    else if( command.Equals( "lift" ) )
    {
        if( command.m_arguments[1] >= -100 && command.m_arguments[1] <= 100 )
        {
            trg_lift = command.m_arguments[1] / 1.0;
            vp = 1500 + 500 * trg_lift;
            vs = vp;
        }
    }
#endif
    else if (command.Equals("lift") || command.Equals("strafe")  ){
      if (command.Equals("lift")){
        if (command.m_arguments[1]>=-100 && command.m_arguments[1]<=100) {
          trg_lift = command.m_arguments[1]/1.0;
          if (trg_lift == 0) {
             if (m_first_zero == 0) {
                 m_first_zero++;
             }
          } else {
             // turn off PIDs if user changes depth
             m_bouyancy.RunPID( 0, 0 );
             m_first_zero = 0;
             m_lift_zero_time = (uint32_t) -1;
          }
          // drive horizontal motors here
          if ((p_trg_lift <= 0.0) && (trg_lift >= 0.0)) {
              // direction change
#ifdef OLD_BOARD
              g_SystemMuxes.SetPath(SCL_MC);
              motor_c->Cmd_SetSpeed(0x0000);
              g_SystemMuxes.SetPath(SCL_MD);
              motor_d->Cmd_SetSpeed(0x0000);
#else
              // Now motors A and B
              g_SystemMuxes.SetPath(SCL_NONE);
              motor_signals->DigitalWriteLow(pca9685::LED_0);
              motor_signals->DigitalWriteLow(pca9685::LED_1);
              delay(10);
#endif
          } else {
              if ((trg_lift <= 0.0) && (p_trg_lift >= 0.0)) {
#ifdef OLD_BOARD
                  g_SystemMuxes.SetPath(SCL_MC);
                  motor_c->Cmd_SetSpeed(0x0000);
                  g_SystemMuxes.SetPath(SCL_MD);
                  motor_d->Cmd_SetSpeed(0x0000);
#else
                  // Now motors A and B
                  g_SystemMuxes.SetPath(SCL_NONE);
                  motor_signals->DigitalWriteLow(pca9685::LED_0);
                  motor_signals->DigitalWriteLow(pca9685::LED_1);
                  delay(10);
#endif
              }
          }
          // -up +down
          if (trg_lift >= 0.0) {
#ifdef OLD_BOARD
              g_SystemMuxes.SetPath(SCL_PWM);
              motor_signals->DigitalWriteLow(pca9685::LED_5);
              motor_signals->DigitalWriteHigh(pca9685::LED_7);
              g_SystemMuxes.SetPath(SCL_MC);
              motor_c->Cmd_SetSpeed(SCALE_SPEED(trg_lift));
              g_SystemMuxes.SetPath(SCL_MD);
              motor_d->Cmd_SetSpeed(SCALE_SPEED(trg_lift));
#else
              // Now motors A and B
              g_SystemMuxes.SetPath(SCL_NONE);
              motor_signals->DigitalWriteHigh(pca9685::LED_5);
              motor_signals->DigitalWriteLow(pca9685::LED_6);
              delay(10);
              motor_signals->DigitalWrite(pca9685::LED_0, ON_TIME(trg_lift), OFF_TIME(trg_lift));
              motor_signals->DigitalWrite(pca9685::LED_1, ON_TIME(trg_lift), OFF_TIME(trg_lift));
#endif
          } else {
#ifdef OLD_BOARD
              g_SystemMuxes.SetPath(SCL_PWM);
              motor_signals->DigitalWriteHigh(pca9685::LED_5);
              motor_signals->DigitalWriteLow(pca9685::LED_7);
              g_SystemMuxes.SetPath(SCL_MC);
              motor_c->Cmd_SetSpeed(SCALE_SPEED(trg_lift));
              g_SystemMuxes.SetPath(SCL_MD);
              motor_d->Cmd_SetSpeed(SCALE_SPEED(trg_lift));
#else
              // Now motors A and B
              g_SystemMuxes.SetPath(SCL_NONE);
              motor_signals->DigitalWriteLow(pca9685::LED_5);
              motor_signals->DigitalWriteHigh(pca9685::LED_6);
              delay(10);
              motor_signals->DigitalWrite(pca9685::LED_0, ON_TIME(trg_lift), OFF_TIME(trg_lift));
              motor_signals->DigitalWrite(pca9685::LED_1, ON_TIME(trg_lift), OFF_TIME(trg_lift));
#endif
          }
          // if we are zero, mark the zero time
          if ((trg_lift == 0) && m_first_zero) {
             m_lift_zero_time = millis();
          }
          p_trg_lift = trg_lift;

          //the vertical component of the thrust factor is
          //vThrust = sin(65)*thrust

          //This spreads the percentage power over the fully available
          //range of the thruster.  Since the current thrusters
          //are less powerfull in the reverse direction and both motors
          //should provide the same thrust, the maximum thurst is
          //governed by the negative modifier of the motor.

          //assume that port and starboard vertical thrusters
          //have the same modifiers for the moment.
          vp = 1500 + 500 * trg_lift;
          vs = 1500 + 500 * trg_lift;
          vp2 = vp;
          vs2 = vs;
          Serial.print("THR2XV2.vp1:");Serial.print(vp);Serial.print(";");
          Serial.print("THR2XV2.vs1:");Serial.print(vs);Serial.print(";");
	  Serial.print( F( "ENDUPDATE:1;" ) );
        }
      }

      else if (command.Equals("strafe")){
        if (command.m_arguments[1]>=-100 && command.m_arguments[1]<=100) {
          trg_strafe = command.m_arguments[1]/1.0;
          // drive horizontal motors here
          if ((p_trg_strafe <= 0.0) && (trg_strafe >= 0.0)) {
              // direction change
#ifdef OLD_BOARD
              g_SystemMuxes.SetPath(SCL_MC);
              motor_c->Cmd_SetSpeed(0x0000);
              g_SystemMuxes.SetPath(SCL_MD);
              motor_d->Cmd_SetSpeed(0x0000);
#else
              // Now motors A and B
              g_SystemMuxes.SetPath(SCL_NONE);
              motor_signals->DigitalWriteLow(pca9685::LED_0);
              motor_signals->DigitalWriteLow(pca9685::LED_1);
              delay(10);
#endif
          } else {
              if ((trg_strafe <= 0.0) && (p_trg_strafe >= 0.0)) {
#ifdef OLD_BOARD
                  g_SystemMuxes.SetPath(SCL_MC);
                  motor_c->Cmd_SetSpeed(0x0000);
                  g_SystemMuxes.SetPath(SCL_MD);
                  motor_d->Cmd_SetSpeed(0x0000);
#else
                  // Now motors A and B
                  g_SystemMuxes.SetPath(SCL_NONE);
                  motor_signals->DigitalWriteLow(pca9685::LED_0);
                  motor_signals->DigitalWriteLow(pca9685::LED_1);
                  delay(10);
#endif
              }
          }
          // -up +down
          // one motor drives the other is off
          if (trg_strafe >= 0.0) {
#ifdef OLD_BOARD
              g_SystemMuxes.SetPath(SCL_PWM);
              motor_signals->DigitalWriteHigh(pca9685::LED_5);
              motor_signals->DigitalWriteHigh(pca9685::LED_7);
              g_SystemMuxes.SetPath(SCL_MC);
              motor_c->Cmd_SetSpeed(SCALE_SPEED(trg_strafe));
              g_SystemMuxes.SetPath(SCL_MD);
              motor_d->Cmd_SetSpeed(SCALE_SPEED(trg_strafe));
#else
              // Now motors A and B
              g_SystemMuxes.SetPath(SCL_NONE);
              motor_signals->DigitalWriteLow(pca9685::LED_5);
              motor_signals->DigitalWriteLow(pca9685::LED_6);
              delay(10);
              motor_signals->DigitalWrite(pca9685::LED_0, ON_TIME(trg_strafe), OFF_TIME(trg_strafe));
              motor_signals->DigitalWrite(pca9685::LED_1, ON_TIME(trg_strafe), OFF_TIME(trg_strafe));
#endif
          } else {
#ifdef OLD_BOARD
              g_SystemMuxes.SetPath(SCL_PWM);
              motor_signals->DigitalWriteLow(pca9685::LED_5);
              motor_signals->DigitalWriteLow(pca9685::LED_7);
              g_SystemMuxes.SetPath(SCL_MC);
              motor_c->Cmd_SetSpeed(SCALE_SPEED(trg_strafe));
              g_SystemMuxes.SetPath(SCL_MD);
              motor_d->Cmd_SetSpeed(SCALE_SPEED(trg_strafe));
#else
              // Now motors A and B
              g_SystemMuxes.SetPath(SCL_NONE);
              motor_signals->DigitalWriteHigh(pca9685::LED_5);
              motor_signals->DigitalWriteHigh(pca9685::LED_6);
              delay(10);
              motor_signals->DigitalWrite(pca9685::LED_0, ON_TIME(trg_strafe), OFF_TIME(trg_strafe));
              motor_signals->DigitalWrite(pca9685::LED_1, ON_TIME(trg_strafe), OFF_TIME(trg_strafe));
#endif
          }
          p_trg_strafe = trg_strafe;

          //strafe (side motion) is limited to whatever thrust is still available
          //from the vertical thruster range.  If vertical is full power,
          //the strafe will be zero.
          maxVtransDelta = std::abs((500.0/std::abs(port_vertical_motor.m_negativeModifier))*(1.0-std::abs(trg_lift)));
          Serial.print("THR2XV2.mvd:");Serial.print(maxVtransDelta);Serial.print(";");
          st = constrain( (int) ((500.0/std::abs(port_vertical_motor.m_negativeModifier))*trg_strafe),-maxVtransDelta,maxVtransDelta);
          Serial.print("THR2XV2.st:");Serial.print(st);Serial.print(";");
          //Adjust the vertrans thrusters with the ideal translate value
          //if we go the wrong way... switch these.
          Serial.print("THR2XV2.vp0:");Serial.print(vp);Serial.print(";");

          vp2=vp+st;
          vs2=vs-st;
          Serial.print("THR2XV2.vp2:");Serial.print(vp2);Serial.print(";");
          Serial.print("THR2XV2.vs2:");Serial.print(vs2);Serial.print(";");
	  Serial.print( F( "ENDUPDATE:1;" ) );
        }
      }

    }

    #ifdef PIN_ENABLE_ESC
    else if( command.Equals( "escp" ) )
    {
#if 0
        escpower.Write( command.m_arguments[1] ); //Turn on the ESCs
        Serial.print( F( "log:escpower=" ) );
        Serial.print( command.m_arguments[1] );
        Serial.print( ';' );
	Serial.print( F( "ENDUPDATE:1;" ) );
#endif
    }
    #endif

    else if( command.Equals( "wake" ) )
    {
#if 0
        // Set greeting state to true and reset timer
        isGreeting = true;
        controltime.Reset();

        // Turn off ESCs
        #ifdef PIN_ENABLE_ESC
            escpower.Write( 0 );
        #else
            port_motor.Deactivate();
            vertical_motor.Deactivate();
            port_motor.Deactivate();
        #endif
#endif
    }

    if( isGreeting )
    {
#if 0
        if( controltime.HasElapsed( greetDelay_ms ) )
        {
            // Turn on ESCs
            #ifdef PIN_ENABLE_ESC
                escpower.Write( 1 );
            #else
                port_motor.Activate();
                vertical_motor.Activate();
                port_motor.Activate();
            #endif

            isGreeting = false;
        }
        else
        {
            return;
        }
#endif
    }

    //to reduce AMP spikes, smooth large power adjustments out. This incrementally adjusts the motors and servo
    //to their new positions in increments.  The increment should eventually be adjustable from the cockpit so that
    //the pilot could have more aggressive response profiles for the ROV.
    if( controltime.HasElapsed( 50 ) )
    {
#if 0
        if (p!=new_p || vp2!=new_vp || vs2!=new_vs || s!=new_s)
        {
            new_p = p;
            new_vp = vp2;
            new_vs = vs2;
            new_s = s;

            // Check to see if any motors are non-neutral to signal system that at least one motor is running
            if( p != MOTOR_TARGET_NEUTRAL_US || vp != MOTOR_TARGET_NEUTRAL_US || vs != MOTOR_TARGET_NEUTRAL_US || s != MOTOR_TARGET_NEUTRAL_US )
            {
                NDataManager::m_thrusterData.MotorsActive = true;
            }
            else
            {
                NDataManager::m_thrusterData.MotorsActive = false;
            }

            Serial.print( F( "motors:" ) );
            Serial.print( port_motor.SetMotorTarget( new_p ) );
            Serial.print( ',' );
            Serial.print( port_vertical_motor.SetMotorTarget( new_vp ) );
            Serial.print( ',' );
            Serial.print( starboard_vertical_motor.SetMotorTarget( new_vs ) );
            Serial.print( ',' );
            Serial.print( starboard_motor.SetMotorTarget( new_s ) );
            Serial.print( ';' );
	    Serial.print( F( "ENDUPDATE:1;" ) );
        }
#endif
    }

    NDataManager::m_navData.FTHR = map( ( new_p + new_s ) / 2, 1000, 2000, -100, 100 );

#if 0
    //The output from the motors is unique to the thruster configuration
    if( thrusterOutput.HasElapsed( 1000 ) )
    {
        Serial.print( F( "mtarg:" ) );
        Serial.print( p );
        Serial.print( ',' );
        Serial.print( vp );
        Serial.print( ',' );
        Serial.print( vs );
        Serial.print( ',' );
        Serial.print( s );
        Serial.print( ';' );
        NDataManager::m_thrusterData.MATC = port_motor.IsActive() || port_motor.IsActive() || port_motor.IsActive();
        Serial.print( F( "mtrmod:" ) );
        Serial.print( port_motor.m_positiveModifier );
        Serial.print( "," );
        Serial.print( port_vertical_motor.m_positiveModifier );
        Serial.print( "," );
        Serial.print( starboard_vertical_motor.m_positiveModifier );
        Serial.print( "," );
        Serial.print( starboard_motor.m_positiveModifier );
        Serial.print( "," );
        Serial.print( port_motor.m_negativeModifier );
        Serial.print( "," );
        Serial.print( port_vertical_motor.m_negativeModifier );
        Serial.print( "," );
        Serial.print( starboard_vertical_motor.m_negativeModifier );
        Serial.print( "," );
        Serial.print( starboard_motor.m_negativeModifier );
        Serial.print( ";" );
	Serial.print( F( "ENDUPDATE:1;" ) );
    }
#endif
}

void CThrusters::Enable(uint8_t value) {
    m_enable = value;
}

void CThrusters::Vertical(int32_t value) {
    trg_lift = value;
    // drive horizontal motors here
    if ((p_trg_lift <= 0.0) && (trg_lift >= 0.0)) {
        // direction change
        // Now motors A and B
        g_SystemMuxes.SetPath(SCL_NONE);
        motor_signals->DigitalWriteLow(pca9685::LED_0);
        motor_signals->DigitalWriteLow(pca9685::LED_1);
        delay(10);
    } else {
        if ((trg_lift <= 0.0) && (p_trg_lift >= 0.0)) {
            // Now motors A and B
            g_SystemMuxes.SetPath(SCL_NONE);
            motor_signals->DigitalWriteLow(pca9685::LED_0);
            motor_signals->DigitalWriteLow(pca9685::LED_1);
            delay(10);
        }
    }
    // -up +down
    if (trg_lift >= 0.0) {
        // Now motors A and B
        g_SystemMuxes.SetPath(SCL_NONE);
        motor_signals->DigitalWriteHigh(pca9685::LED_5);
        motor_signals->DigitalWriteLow(pca9685::LED_6);
        delay(10);
        motor_signals->DigitalWrite(pca9685::LED_0, ON_TIME(trg_lift), OFF_TIME(trg_lift));
        motor_signals->DigitalWrite(pca9685::LED_1, ON_TIME(trg_lift), OFF_TIME(trg_lift));
    } else {
        // Now motors A and B
        g_SystemMuxes.SetPath(SCL_NONE);
        motor_signals->DigitalWriteLow(pca9685::LED_5);
        motor_signals->DigitalWriteHigh(pca9685::LED_6);
        delay(10);
        motor_signals->DigitalWrite(pca9685::LED_0, ON_TIME(trg_lift), OFF_TIME(trg_lift));
        motor_signals->DigitalWrite(pca9685::LED_1, ON_TIME(trg_lift), OFF_TIME(trg_lift));
    }
    // drive both motors together in the same direction
    p_trg_lift = trg_lift;
}
#endif
