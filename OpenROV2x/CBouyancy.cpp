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
   m_wasRunning = 0;
}

void CBouyancy::Initialize()
{
   // leave all user generated motor and bouyancy control operations enabled

   m_thrusters.Enable(1);
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
   int32_t newBallast = 0;

   if( NCommManager::m_isCommandAvailable ) {
      if (commandIn.Equals( "start_balance" )) { 
           m_startUp = commandIn.m_arguments[1]; 
           Serial.print( F( "cbalance:" ) );
           Serial.print( m_startUp );
           Serial.print( ';' );
           Serial.print( F( "ENDUPDATE:1;" ) );
      } else {
          if (commandIn.Equals( "enable_thrusters" )) { 
              uint32_t value =  commandIn.m_arguments[1];
              m_thrusters.Enable(value);
          } else {
              if (commandIn.Equals( "enable_pids" )) { 
                  uint32_t value = commandIn.m_arguments[1];
                  m_motorPID = value&0x01;
                  m_ballastPID = value&0x02;
              } else {
                  if (commandIn.Equals( "init_balance" )) { 
                      uint32_t value = commandIn.m_arguments[1];
                      m_initialization = value;
                  }
              }
          }
      }
   }

   if (m_startUp == 0) {
      if (m_wasRunning > 0) {
         m_wasRunning = 0;
         m_ballast.Drive(0);
         m_thrusters.Vertical(0);
         m_thrusters.Enable(1);
      }
      return;
   }

   m_wasRunning = 1;

   if (m_initialization) {
       // 0.3 meters is about 1 foot
       m_TargetDepth = 0.3;
       // turn off thruster control
       m_thrusters.Enable(0);
#if 0
       // The initialization code
       m_thrusters.Vertical( m_newMotor );
       m_ballast.Drive( newBallast );
       delay(250);
       m_ballast.Drive(0);
       // while the depth is not yet at about a foot or so
       while (m_p86bsd030pa.GetDepth() < m_TargetDepth) {
          // run the motor to dive
          delay(10);
       }
#endif
       m_initialization = 0;
   }

   // check if the pid file is there
   // read it if it is
   FILE *fp = fopen("/home/pi/pid.txt","r");
   if (fp) {
      fscanf(fp, "%lf\n", &m_KPM );
      fscanf(fp, "%lf\n", &m_KIM );
      fscanf(fp, "%lf\n", &m_KDM );
      fscanf(fp, "%lf\n", &m_KPB );
      fscanf(fp, "%lf\n", &m_KIB );
      fscanf(fp, "%lf\n", &m_KDB );
      fclose(fp);
      printf( "PM: %lf\n", m_KPM );
      printf( "IM: %lf\n", m_KIM );
      printf( "DM: %lf\n", m_KDM );
      printf( "PB: %lf\n", m_KPB );
      printf( "IB: %lf\n", m_KIB );
      printf( "DB: %lf\n", m_KDB );
   }

   // now we are around a foot
   // run the PID to manage the ballast and slow the motor speed
   float actualDepth = RunMotor();
   Check(actualDepth);

   // about every 10 seconds
   m_nowB = millis();
   if ((m_ballastPID) && ((m_nowB - m_startB) > 10000)) {
       newBallast = m_newMotor/m_KDB; // BallastControlPID( 0, m_newMotor, m_nowB-m_startB );
       // Number of 1/4 second intervals to run the ballast pump
       // sign indicates direction
       if (std::abs(newBallast) > 1.0) {
           int32_t drive = ((newBallast > 0) ? 50 : -50);
           newBallast = std::abs(std::round(newBallast)); 
           uint32_t turns = 0;
           // break it into 1/4 second intervals to force check pressure
           while (turns < (uint32_t)newBallast) {
                // Check if the maximum pressure is exceeded
                if (m_ballast.Drive( drive )) break;
                delay(250);
                actualDepth = RunMotor();
                if (Check(actualDepth)) break;
                turns++;
           }
           m_ballast.Drive( 0 );
       }
       oldBallast = newBallast;
       m_startB = millis();
   }

   Check(actualDepth);
}

uint8_t CBouyancy::Check( float actualDepth ) {
   // if we are close to the target depth
   // (about 6 inches or so)
   // and the motor is slow then allow the user
   // to control things again
   //
   if ((std::abs(m_TargetDepth - actualDepth) < 0.15) &&
       (std::abs(m_newMotor) < 10)) {
       m_thrusters.Vertical( 0 );
       m_thrusters.Enable(1);
       return(1);
   }
   return(0);
}

float CBouyancy::RunMotor() {
   float actualDepth = m_p86bsd030pa.GetDepth();
   // about every 50 msec (but motors won't respond if every 20 msec or so)
   m_nowM = millis();
   printf("Now: %d msec\n",m_nowM);
   if ((m_motorPID)  && ((m_nowM - m_startM) > 50)) {
       m_newMotor = MotorControlPID( m_TargetDepth, actualDepth, m_nowM-m_startM );
       m_thrusters.Vertical( m_newMotor );
       m_startM = millis();
   }
   return( actualDepth );
}

// 
// This PID is used to control vertical movement and maintain depth
// DT is the time since the last call to the PID
double CBouyancy::MotorControlPID( double targetDepth, double actualDepth, double DT ) {
    static double p_p_err = 0, p_i_err = 0;
    double MotorValue, p_err = 0, i_err = 0, d_err = 0;
    double i_term = 0;

    p_err = targetDepth - actualDepth;

    DT /= 100;
    i_err = p_err + p_i_err; 
    if (i_err > 100) i_err = 100;
    if (i_err < -100) i_err = -100;
    i_term = m_KIM*i_err*DT;
    if (i_term > 100) i_term = 100;
    if (i_term < -100) i_term = -100;

    d_err = p_err - p_p_err; 
    p_p_err = p_err;
    p_i_err = i_err;
    
    MotorValue = m_KPM*p_err + i_term + m_KDM*d_err/DT;
    printf("Motor p_err: %f i_err %f i_term %f d_err %f DT %f Motor %f\n",
            p_err, i_err, i_term, d_err, DT, MotorValue );
    if (MotorValue >  100) MotorValue = 100;
    if (MotorValue < -100) MotorValue = -100;
    return(MotorValue);
}
//
// This PID is used to control the ballast motor
// and used to acheive balance
// DT is the time since the last call to the PID
double CBouyancy::BallastControlPID( double targetMotor, double actualMotor, double DT ) {
    static double p_p_err = 0, p_i_err = 0;
    double MotorTurnTime, i_term = 0, p_err = 0, i_err = 0, d_err = 0;

    DT /= 10000;
    p_err = targetMotor - actualMotor;
    i_err = p_err + p_i_err; 
    if (i_err > 25) i_err = 25;
    if (i_err < -25) i_err = -25;
    i_term = m_KIB*i_err*DT;
    if (i_term > 25) i_term = 25;
    if (i_term < -25) i_term = -25;

    d_err = p_err - p_p_err; 
    p_p_err = p_err;
    p_i_err = i_err;
    
    MotorTurnTime = m_KPB*p_err + i_term + m_KDB*d_err/DT;
    if (MotorTurnTime >  25) MotorTurnTime = 25;
    if (MotorTurnTime < -25) MotorTurnTime = -25;
    return(MotorTurnTime);
}

void CBouyancy::SetTargetDepth( float targetDepth ) {
    m_TargetDepth = targetDepth;
}

uint8_t CBouyancy::BouyancyActive( void ) {
    return( m_startUp );
}

void CBouyancy::RunPID( uint8_t motor, uint8_t ballast ) {
    m_motorPID = motor;
    m_ballastPID = ballast;
}
#endif
