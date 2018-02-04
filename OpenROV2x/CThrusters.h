#pragma once

// Includes
#include "CModule.h"

class CThrusters : public CModule
{
public:
    static const int kMotorCount;

    void Initialize();
    void Update( CCommand& commandIn );
    void Vertical( int32_t value );
    void Enable( uint8_t value );
private:
    uint8_t m_enable;
};
