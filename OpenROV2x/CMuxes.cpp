#include "SysConfig.h"
#if(HAS_PCA9547)

#include "PCA9547.h"
#include "CMuxes.h"

using namespace pca9547;

namespace
{
}

CMuxes::CMuxes()
{
    m_device0 = new pca9547::PCA9547( i2cInterface0, pca9547::PCA9547_0_ADDRESS );
    m_device1 = new pca9547::PCA9547( i2cInterface1, pca9547::PCA9547_1_ADDRESS );
    m_device2 = new pca9547::PCA9547( i2cInterface2, pca9547::PCA9547_2_ADDRESS );
}

void CMuxes::Initialize()
{
    m_device0->Initialize();
    m_device1->Initialize();
    m_device2->Initialize();
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
#endif
