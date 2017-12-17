#include "SysConfig.h"
#if(HAS_PCA9547)

#include "PCA9547.h"
#include "PCA9539.h"
#include "CMuxes.h"

extern I2C I2C0;

using namespace pca9547;

namespace
{
}

CMuxes::CMuxes()
{
    m_device0 = new pca9547::PCA9547( &I2C0, pca9547::PCA9547_0_ADDRESS );
    m_device1 = new pca9547::PCA9547( &I2C0, pca9547::PCA9547_1_ADDRESS );
    m_device2 = new pca9547::PCA9547( &I2C0, pca9547::PCA9547_2_ADDRESS );
    m_dio1 = new pca9539::PCA9539( &I2C0 );
    m_dio2 = new pca9539::PCA9539( &I2C0 );
}

void CMuxes::Initialize()
{
    m_device0->Initialize();
    m_device1->Initialize();
    m_device2->Initialize();
    SetPath(SCL_DIO1);
    m_dio1->PinMode(0xEAF6);
    m_dio1->DigitalWrite(0,0x01);
    m_dio1->DigitalWrite(3,0x01);
    m_dio1->DigitalWrite(8,0x01);
    m_dio1->DigitalWrite(10,0x01);
    m_dio1->DigitalWrite(12,0x01);
    SetPath(SCL_DIO2);
    m_dio2->PinMode(0xFF00);
    m_dio2->DigitalWrite(0,0x01);
    m_dio2->DigitalWrite(1,0x00);
    m_dio2->DigitalWrite(2,0x00);
    m_dio2->DigitalWrite(3,0x00);
    m_dio2->DigitalWrite(4,0x00);
    m_dio2->DigitalWrite(5,0x00);
    m_dio2->DigitalWrite(6,0x00);
    m_dio2->DigitalWrite(7,0x00);
}

void CMuxes::Update( CCommand &commandIn )
{
}

void CMuxes::SetPath( uint16_t path )
{
    switch(path) {
   // 0x70 CH0 through CH7
        case SCL_12V_RPA:
        case SCL_12V_RPB:
        case SCL_3V3_SYS:
        case SCL_5V_SYS:
        case SCL_IMU0:
        case SCL_UNUSED0:
        case SCL_UNUSED1:
        case SCL_UNUSED2:
           m_device0->DigitalWrite( (path%8)+pca9547::PCA9547_SELECT::CHANNEL_0 );
           m_device1->DigitalWrite( pca9547::PCA9547_SELECT::NO_CHANNEL );
           m_device2->DigitalWrite( pca9547::PCA9547_SELECT::NO_CHANNEL );
           break;
    // 0x71 CH0 through CH7
        case SCL_BATT:
        case SCL_CRIT:
        case SCL_CHG:
        case SCL_DIO2:
        case SCL_ME:
        case SCL_BAL:
        case SCL_PXDCR:
        case SCL_EXP:
           m_device1->DigitalWrite( (path%8)+pca9547::PCA9547_SELECT::CHANNEL_0 );
           m_device0->DigitalWrite( pca9547::PCA9547_SELECT::NO_CHANNEL );
           m_device2->DigitalWrite( pca9547::PCA9547_SELECT::NO_CHANNEL );
           break;
    // 0x72 CH0 through CH7
        case SCL_MA:
        case SCL_MB:
        case SCL_MC:
        case SCL_MD:
        case SCL_HZT:
        case SCL_VRT:
        case SCL_PWM:
        case SCL_DIO1:
           m_device2->DigitalWrite( (path%8)+pca9547::PCA9547_SELECT::CHANNEL_0 );
           m_device0->DigitalWrite( pca9547::PCA9547_SELECT::NO_CHANNEL );
           m_device1->DigitalWrite( pca9547::PCA9547_SELECT::NO_CHANNEL );
           break;
        default:
           break;
    }

}

void CMuxes::WriteExtendedGPIO( uint16_t pin, uint8_t state )
{
    switch(pin) {
        // DIO1
        case MAB_EN:
        case MAB_FLT:
        case MAB_ALT:
        case MCD_EN:
        case MCD_FLT:
        case MCD_ALT:
        case BAL_FLT:
        case BAL_ALT:
        case VALVE_1_EN:
        case VALVE_1_STAT:
        case VALVE_2_EN:
        case VALVE_2_STAT:
        case VALVE_3_EN:
        case VALVE_3_STAT:
        case LEAK_SW:
        case UNUSED:
           SetPath(SCL_DIO1);
           m_dio1->DigitalWrite((pin%16), state);
           break;
        // DIO2
        case BAL_LEDS_EN:
        case RLY_SAF:
        case RLY_ARM:
        case STAT_LED1:
        case STAT_LED2:
        case STAT_LED3:
        case RPI_LINX_EN:
        case RPI_LINX_KEY:
        case CRIT_ALT:
        case CRIT_FLT:
        case BATT_FLT:
        case CHG_ALT:
        case CHG_DETECT:
        case RLY_STAT:
        case BLOW_VALVE_STAT:
        case MAG_SW_MON:
           SetPath(SCL_DIO2);
           m_dio2->DigitalWrite((pin%16), state);
           break;
        default:
           break;
    }
}

uint8_t CMuxes::ReadExtendedGPIO( uint16_t pin )
{
    uint8_t state = 0x00;
    switch(pin) {
        // DIO1
        case MAB_EN:
        case MAB_FLT:
        case MAB_ALT:
        case MCD_EN:
        case MCD_FLT:
        case MCD_ALT:
        case BAL_FLT:
        case BAL_ALT:
        case VALVE_1_EN:
        case VALVE_1_STAT:
        case VALVE_2_EN:
        case VALVE_2_STAT:
        case VALVE_3_EN:
        case VALVE_3_STAT:
        case LEAK_SW:
        case UNUSED:
           SetPath(SCL_DIO1);
           m_dio1->DigitalRead((pin%16), &state);
           break;
        // DIO2
        case BAL_LEDS_EN:
        case RLY_SAF:
        case RLY_ARM:
        case STAT_LED1:
        case STAT_LED2:
        case STAT_LED3:
        case RPI_LINX_EN:
        case RPI_LINX_KEY:
        case CRIT_ALT:
        case CRIT_FLT:
        case BATT_FLT:
        case CHG_ALT:
        case CHG_DETECT:
        case RLY_STAT:
        case BLOW_VALVE_STAT:
        case MAG_SW_MON:
           SetPath(SCL_DIO2);
           m_dio2->DigitalRead((pin%16), &state);
           break;
        default:
           break;
    }


    return( state );

}
#endif
