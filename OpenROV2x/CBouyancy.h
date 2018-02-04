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

private:
    const int32_t m_KPM = 2;
    const int32_t m_KIM = 1;
    const int32_t m_KDM = 0;
    const int32_t m_KPB = 2;
    const int32_t m_KIB = 1;
    const int32_t m_KDB = 0;

    float m_TargetDepth = 0.3;
    uint8_t m_initialization;
    uint8_t m_startUp;
    uint8_t m_motorPID;
    uint8_t m_ballastPID;

    double BallastControlPID( double targetDepth, double actualDepth, double DT );
    double MotorControlPID( double targetDepth, double actualDepth, double DT );

};

