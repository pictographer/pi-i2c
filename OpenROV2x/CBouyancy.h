#pragma once

// Includes
#include "PCA9685.h"
#include "DRV10983.h"
#include "CModule.h"

class CBouyancy : public CModule
{
public:
    CBouyancy();
    void Initialize();
    void Update( CCommand& commandIn );
    void SetTargetDepth( float targetDepth );
    void RunPID( uint8_t motor, uint8_t ballast );
    float RunMotor( void ); 

private:
    const double m_KPM = 50.0;
    const double m_KIM = 5.0;
    const double m_KDM = 0.05;
    const double m_KPB = 0.1;
    const double m_KIB = 0.01;
    const double m_KDB = 0.01;

    int32_t m_newMotor = 10;
    uint32_t m_startM = 0, m_nowM = 0, m_startB = 0, m_nowB = 0;
    float m_TargetDepth = 0.3;
    uint8_t m_initialization;
    uint8_t m_startUp;
    uint8_t m_motorPID;
    uint8_t m_ballastPID;
    uint8_t m_wasRunning;

    double BallastControlPID( double targetDepth, double actualDepth, double DT );
    double MotorControlPID( double targetDepth, double actualDepth, double DT );

};

