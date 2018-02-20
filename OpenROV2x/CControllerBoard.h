#pragma once

// Includes
#include <I2C.h>
#include "CModule.h"
#include "INA260.h"
#include "BQ34Z100.h"

#define RPA 0
#define RPB 1

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
    bq34z100::BQ34Z100 *m_chargeSense;
    long readCharge();
    long readVcc();
    long readLeakDetector();
    float read20Volts();
    float readPiCurrent( uint8_t thePI );
    void LoadFsFile( bq34z100::BQ34Z100 *chargeSense, const char *filename );
};
