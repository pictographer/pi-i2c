#pragma once

// Includes
#include <PCA9547.h>
#include "CCommand.h"
#include "CModule.h"

enum EPathSelect : uint16_t
{
    // 0x70 CH0 through CH7
    SCL_12V_RPA = 0x0000,
    SCL_12V_RPB,
    SCL_3V3_SYS,
    SCL_5V_SYS,
    SCL_IMU0,
    SCL_UNUSED0,
    SCL_UNUSED1,
    SCL_UNUSED2,
    // 0x71 CH0 through CH7
    SCL_BATT,
    SCL_CRIT,
    SCL_CHG,
    SCL_DIO2,
    SCL_ME,
    SCL_BAL,
    SCL_PXDCR,
    SCL_EXP,
    // 0x72 CH0 through CH7
    SCL_MA,
    SCL_MB,
    SCL_MC,
    SCL_MD,
    SCL_HZT,
    SCL_VRT,
    SCL_PWM,
    SCL_DIO1
};

class CMuxes : public CModule
{
public:
	CMuxes();
	void Initialize();
	void Update( CCommand &commandIn );
        void SetPath( uint16_t path );

private:
	// Device driver
	pca9547::PCA9547 *m_device0;
	pca9547::PCA9547 *m_device1;
	pca9547::PCA9547 *m_device2;

};



