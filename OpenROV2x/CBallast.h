#pragma once

// Includes
#include "PCA9539.h"
#include "PCA9685.h"
#include "DRV10983.h"
#include "CModule.h"
#include <orutil.h>

class CBallast : public CModule
{
public:
    CBallast();
    void Initialize();
    void Update( CCommand& commandIn );

private:
    pca9539::PCA9539   *m_power_blow;
    pca9539::PCA9539   *m_valves;
    pca9685::PCA9685   *m_ballast_pwm;
    // source: nsr1215_2.csv
    uint8_t params[12] = {
        0xEA,
        0xC9,
        0x2A,
        0x04,
        0xC0,
        0xFA,
        0x16,
        0xB0,
        0x0F,
        0xA8,
        0x0E,
        0x0C
   };

    // motor controller
    // E - ballast system
    drv10983::DRV10983 *motor_e;

    float       m_targetPower = 0.0f;
    float       m_currentPower = 0.0f;

    uint32_t    m_lastPower_an = 0;
    uint32_t    m_targetPower_an = 0;
    uint32_t    m_currentPower_an = 0;
};

