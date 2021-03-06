#pragma once

// Includes
#include <I2C.h>
#include <limits.h>
#include <stdio.h>
#include "CModule.h"
#include "INA260.h"
#include "BQ34Z100.h"

#define RPA 0
#define RPB 1
#define SECRET_LOG_FILENAME ((char *) "/home/pi/logs/stats.log")
#define SECRET_LOG_FILENAME_SV ((char *) "/home/pi/logs/stats.log.sv")
#define BQ_FS_FILENAME ((char *) "/home/pi/pi-i2c/bq.fs")
#define DF_FS_FILENAME ((char *) "/home/pi/pi-i2c/df.fs")
#define DF_FS_LOADED_FILENAME ((char *) "/home/pi/pi-i2c/df.loaded.fs")

class CControllerBoard : public CModule
{
public:
    // Attributes
    CControllerBoard();
    // Methods
    virtual void Initialize();
    virtual void Update( CCommand& commandIn );
    void SecretReportLine( void );
private:
    const int m_s_max = 300;

    ina260::INA260 *m_powerSense;
    bq34z100::BQ34Z100 *m_chargeSense;

    char m_hostname[HOST_NAME_MAX];

    FILE *m_fp;
    int m_line;

    int m_s_idx;
    int m_use_max;
    float *m_samples;
    float m_boardTemperature;

    float readCharge();
    float estimateCharge();
    long readVcc();
    long readLeakDetector();
    float read20Volts();
    float readPiCurrent( uint8_t thePI );
    float getAverage( float value );
    uint8_t LoadFsFile( bq34z100::BQ34Z100 *chargeSense, const char *filename );
    float readTemp();
    float readCurrent( int pin );
    float readBrdCurrent( int pin );
};
