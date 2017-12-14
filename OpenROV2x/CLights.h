#pragma once

// Includes
#include "CModule.h"
#include "CPin.h"
#include <orutil.h>
#include "PCA9539.h"

class CLights : public CModule
{
public:
    CLights( uint32_t pinIn );
    void Initialize();
    void Update( CCommand& commandIn );

private:
    orutil::CTimer      m_controlTimer;
    orutil::CTimer      m_telemetryTimer;
    CPin        m_pin;

    pca9539::PCA9539   *m_power;
    float       m_targetPower = 0.0f;
    float       m_currentPower = 0.0f;

    uint32_t    m_lastPower_an = 0;
    uint32_t    m_targetPower_an = 0;
    uint32_t    m_currentPower_an = 0;
};

