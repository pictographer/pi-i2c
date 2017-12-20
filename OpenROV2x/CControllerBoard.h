#pragma once

// Includes
#include <I2C.h>
#include "CModule.h"
#include "INA260.h"

class CControllerBoard : public CModule
{
public:
    // Attributes
    CControllerBoard();
    // Methods
    virtual void Initialize();
    virtual void Update( CCommand& commandIn );
private:
    ina260::INA260 *m_powerSense;
    long readVcc();
    float read20Volts();
};
