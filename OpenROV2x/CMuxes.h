#pragma once

// Includes
#include <PCA9547.h>
#include <PCA9539.h>
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
    SCL_TEMP,
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

enum EPinSelect : uint16_t
{
    // DIO1
    MAB_EN = 0x0000,
    MAB_FLT,
    MAB_ALT,
    MCD_EN,
    MCD_FLT,
    MCD_ALT,
    BAL_FLT,
    BAL_ALT,
    VALVE_1_EN,
    VALVE_1_STAT,
    VALVE_2_EN,
    VALVE_2_STAT,
    VALVE_3_EN,
    VALVE_3_STAT,
    LEAK_SW,
    UNUSED,
    // DIO2
    BAL_LEDS_EN,
    RLY_SAF,
    RLY_ARM,
    STAT_LED1,
    STAT_LED2,
    STAT_LED3,
    RPI_LINX_EN,
    RPI_LINX_KEY,
    CRIT_ALT,
    CRIT_FLT,
    BATT_FLT,
    CHG_ALT,
    CHG_DETECT,
    RLY_STAT,
    BLOW_VALVE_STAT,
    MAG_SW_MON
}; 


class CMuxes : public CModule
{
public:
	CMuxes();
	void Initialize();
	void Update( CCommand &commandIn );
        void SetPath( uint16_t path );
        void WriteExtendedGPIO( uint16_t pin, uint8_t state );
        uint8_t ReadExtendedGPIO( uint16_t pin );

private:
	// Device driver
	pca9547::PCA9547 *m_device0;
	pca9547::PCA9547 *m_device1;
	pca9547::PCA9547 *m_device2;
        pca9539::PCA9539 *m_dio1;
        pca9539::PCA9539 *m_dio2;
        uint8_t m_Initialized;

};



