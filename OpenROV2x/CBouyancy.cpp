#include "SysConfig.h"
#if(HAS_BOUYANCY)

// Includes
#include <cmath>

#include "CBouyancy.h"
#include "CBallast.h"
#include "CThrusters.h"
#include "NCommManager.h"
#include "CP86BSD030PA.h"

extern CP86BSD030PA m_p86bsd030pa;
extern CBallast m_ballast;
extern CThrusters m_thrusters;

namespace
{
}

CBouyancy::CBouyancy()
{
   m_initialization = 1;
   m_TargetDepth = 0.3;
   m_motorPID = 1;
   m_ballastPID = 1;
   m_startUp = 0;
}

void CBouyancy::Initialize()
{
   // disable all user generated motor and bouyancy control operations

   m_thrusters.Enable(0);
   // use motors to drop to about 1 foot of depth

   // execute PI loop to hold at about 1 foot

   // start using ballast to acheive balance while
   // slowing the motors
   // slow motor 5% 
   // check depth
   // modify ballast
   // check depth
   // keep looping until motor is at 1%
   // keep running PID to hold position
   
}

void CBouyancy::Update( CCommand& commandIn )
{
   static int32_t oldBallast = 1;
   uint32_t startM = 0, nowM = 0, startB = 0, nowB = 0;
   int32_t newMotor = 30;
   int32_t newBallast = 0;

   if( NCommManager::m_isCommandAvailable ) {
       if (commandIn.Equals( "balance" )) { 
           m_startUp = commandIn.m_arguments[1]; 
      }
   }

   if (m_startUp == 0) return;

   if (m_initialization) {
       // The initialization code
       m_thrusters.Vertical( newMotor );
       m_ballast.Drive( newBallast );
       // while the depth is not yet at about a foot or so
       while (m_p86bsd030pa.GetDepth() < 0.3) {
          // run the motor to dive
          delay(10);
       }
       m_initialization = 0;
   }

   // now we are around a foot
   // run the PID to manage the ballast and slow the motor speed
   float targetDepth = m_TargetDepth;
   float actualDepth = m_p86bsd030pa.GetDepth();

   // as fast as possible (but motors won't respond if every 20 msec or so)
   if (m_motorPID) {
       nowM = millis();
       newMotor = MotorControlPID( targetDepth, actualDepth, nowM-startM );
       m_thrusters.Vertical( newMotor );
       startM = nowM;
   }

   // about every 30 seconds
   nowB = millis();
   if ((m_ballastPID) && ((nowB - startB) > 30000)) {
       newBallast = BallastControlPID( 0, oldBallast, nowB-startB );
       // Number of 1/4 second intervals to run the ballast pump
       // sign indicates direction
       if (newBallast > 1.0) {
           m_ballast.Drive( 50 );
           delay(250*newBallast);
           m_ballast.Drive( 0 );
       }
       oldBallast = newBallast;
       startB = nowB;
   }

   // if vertical thruster not being used
   // keep at current depth using slow motors
   //
   if (std::abs(targetDepth - actualDepth) < 0.05) {
       m_thrusters.Enable(1);
   }
}

// 
// This PID is used to control vertical movement and maintain depth
// DT is the time since the last call to the PID
double CBouyancy::MotorControlPID( double targetDepth, double actualDepth, double DT ) {
    static double p_p_err = 0, p_i_err = 0;
    double MotorValue, p_err = 0, i_err = 0, d_err = 0;

    p_err = targetDepth - actualDepth;
    i_err = p_err + p_i_err; 
    d_err = p_err - p_p_err; 
    p_p_err = p_err;
    p_i_err = i_err;
    
    MotorValue = m_KPM*p_err + m_KIM*i_err*DT + m_KDM*d_err/DT;
    if (MotorValue >  100) MotorValue = 100;
    if (MotorValue < -100) MotorValue = -100;
    return(MotorValue);
}
//
// This PID is used to control the ballast motor
// and used to acheive balance
// DT is the time since the last call to the PID
double CBouyancy::BallastControlPID( double targetDepth, double actualDepth, double DT ) {
    static double p_p_err = 0, p_i_err = 0;
    double MotorValue, p_err = 0, i_err = 0, d_err = 0;

    p_err = targetDepth - actualDepth;
    i_err = p_err + p_i_err; 
    d_err = p_err - p_p_err; 
    p_p_err = p_err;
    p_i_err = i_err;
    
    MotorValue = m_KPB*p_err + m_KIB*i_err*DT + m_KDB*d_err/DT;
    if (MotorValue >  100) MotorValue = 100;
    if (MotorValue < -100) MotorValue = -100;
    return(MotorValue);
}

void CBouyancy::SetTargetDepth( float targetDepth ) {
    m_TargetDepth = targetDepth;
}

void CBouyancy::RunPID( uint8_t motor, uint8_t ballast ) {
    m_motorPID = motor;
    m_ballastPID = ballast;
}
#endif
