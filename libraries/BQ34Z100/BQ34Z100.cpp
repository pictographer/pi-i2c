/*
 * BQ34Z100 library for reading / writing / settting up
 * This allows you to use this ic in your own projects :)
 * By Ben V. Brown (Ralim@Ralimtek.com)
 */

extern "C" {
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
}

#include "BQ34Z100.h"
using namespace bq34z100;

uint8_t flashbytes[32] = {0};
uint8_t addr_flashbytes[33] = {0};

BQ34Z100::BQ34Z100( I2C* i2cInterfaceIn )
{
	m_pI2C = i2cInterfaceIn; 
	m_i2cAddress = bq34z100::BQ34Z100_ADDRESS;
        m_calibrationMode = 0;
}


void BQ34Z100::setI2CAddress( uint8_t address ) {
        // the ds file has the I2C address shifted left 1 bit
        m_i2cAddress = (address>>1);
}

int32_t BQ34Z100::WriteWord( uint8_t addressIn, uint16_t dataIn )
{
        return (int32_t) m_pI2C->WriteRegisterWord( m_i2cAddress, (uint8_t) addressIn, dataIn );
}

int32_t BQ34Z100::WriteByte( uint8_t addressIn, uint8_t dataIn )
{
        return (int32_t)m_pI2C->WriteRegisterByte( m_i2cAddress, (uint8_t)addressIn, dataIn );
}

int32_t BQ34Z100::WriteRegisterBytes( uint8_t addressIn, uint8_t *dataIn, uint8_t numberBytesIn )
{
        return (int32_t)m_pI2C->WriteRegisterBytes( m_i2cAddress, (uint8_t)addressIn, dataIn, numberBytesIn );
}

int32_t BQ34Z100::WriteNBytes( uint8_t *dataOut, uint8_t numberBytesOut ) 
{
        return (int32_t) m_pI2C->WriteBytes( m_i2cAddress, dataOut, numberBytesOut );
}

int32_t BQ34Z100::ReadByte( uint8_t addressIn, uint8_t &dataOut )
{
        return (int32_t)m_pI2C->ReadRegisterByte( m_i2cAddress, (uint8_t)addressIn, &dataOut );
}

int32_t BQ34Z100::ReadNBytes( uint8_t addressIn, uint8_t* dataOut, uint8_t byteCountIn )
{
    return (int32_t)m_pI2C->ReadRegisterNBytes( m_i2cAddress, (uint8_t)addressIn, dataOut, byteCountIn );
}

//read a standard normal access value (the common ones you need)
// assumes no more than a 4 byte access
uint32_t BQ34Z100::Read(uint8_t add, uint8_t length)
{
    uint8_t  dataOut[4];
    uint32_t returnVal = 0;

    ReadNBytes( (uint8_t) add, (uint8_t *) dataOut, length );
    for (int i = 0; i < length; i++)
    {
        returnVal = returnVal + (dataOut[i] << (8 * i));
    }
    return returnVal;
}

int BQ34Z100::enableCal()
{
    (m_calibrationMode == 0 ? m_calibrationMode++ : m_calibrationMode--);
    return readControl(0x2D,0x00);//reading this will toggle calibraion mode
}

int BQ34Z100::enterCal()
{
    m_calibrationMode++;
    return readControl(0x81,0x00);//reading this will enter calibration mode
}


int BQ34Z100::exitCal()
{
    m_calibrationMode--;
    return readControl(0x80,0x00);//reading this will exit calibration mode
}

void BQ34Z100::seal()
{
    uint16_t data = 0x2000;
    WriteWord( 0x00, data );
}

int BQ34Z100::enableIT()
{
    return readControl(0x21,0x00);//reading this will enable the IT algorithm
}
//Read a register set from the control register (used to read the instant current
//  or enable IT)
int BQ34Z100::readControl(uint8_t add,uint8_t add2)
{
    uint16_t additional_data = (add<<8);

    WriteWord( add2, additional_data );

    uint8_t reg_add = 0x00;
    uint32_t temp = Read( reg_add, 2);
    return temp;
}

//returns the current temp
int BQ34Z100::getTemp()
{
    return Read(0x0c, 2) - 2731; //return temp in x10 format
}

uint8_t BQ34Z100::getSOC()
{
    return Read(0x02, 2) ; //return temp in x10 format
}

int BQ34Z100::getVoltage()
{
    return Read(0x08, 2) ; //return temp in x10 format
}

int BQ34Z100::getCapacity()
{
    return Read(0x06, 2) ; //return temp in x10 format
}
int BQ34Z100::getRemaining()
{
    return Read(0x04, 2) ; //return temp in x10 format
}
int BQ34Z100::getCurrent()
{
    return Read(0x0a, 2) ; //return temp in x10 format
}
int BQ34Z100::getStatus()
{
    return readControl(0x00,0x00);
}
int BQ34Z100::getFlags()
{
    return Read(0x0E, 2) ; //
}
void BQ34Z100::reset()
{
    WriteWord( 0x00, 0x41 );
}

/**    FLASH CONFIGURATION SETTINGS  **/
int BQ34Z100::setup(uint8_t BatteryChemistry,uint8_t SeriesCells,uint16_t CellCapacity,uint16_t PackCurrentVoltage,uint16_t AppliedCurrent)
{
    //so we set the following:
    /*  Battery Chemistry
    *   Number of Series Cells
    *   Cell capacity (mAh)
    *   Set to use internal Temp Sensor
    *   Disable built in voltage divider
    *   Calibrate voltage Divider
    *   Calibrate Current Shunt
    */
    //Set chemistry

    //Set Series Cells
    //Set cell capacity
    chg48Table(CellCapacity,CellCapacity,4200,4200,4200,4200);
    //set pack configuration
    chg64Table(0x29c0,0,0,SeriesCells);
    if(PackCurrentVoltage>5000)
    {
        //calibrate VDivider
        CalibrateVoltageDivider(PackCurrentVoltage);
        CalibrateVoltageDivider(PackCurrentVoltage);//calling 3 times due to rounding issues
        CalibrateVoltageDivider(PackCurrentVoltage);//it gets closer each time
    }
    // we now need to calibrate thecurrent
    CalibrateCurrentShunt(AppliedCurrent);

}
void BQ34Z100::CalibrateCurrentShunt(int16_t current)
{
    if(current>-200 && current<200)
        return;//too small to use to calibrate
    //current is in milliamps
    if(current<0)
        current=-current;
    int16_t currentReading = getCurrent();
    if(currentReading<0)
        currentReading = -currentReading;
    if(currentReading==0)
        currentReading=20;
    readFlash(0x68, 15);
    delay(30);
  
    uint32_t curentGain = ((uint32_t)flashbytes[0])<<24 | ((uint32_t)flashbytes[1])<<16|((uint32_t)flashbytes[2])<<8|(uint32_t)flashbytes[3];
    float currentGainResistance = (4.768/XemicsTofloat(curentGain));
    float newGain = (((float)currentReading)/((float)current)) * currentGainResistance;
    //we now have the new resistance calculated
    chg104Table(0,newGain,newGain);
    //chg104Table(0,5,5);

    delay(30);
}

uint16_t BQ34Z100::CalibrateVoltageDivider(uint16_t currentVoltage)
{
    if(currentVoltage<5000)
        return 0;
//So do this we set the voltage divider to 1.5 Times the current pack voltage (to start with)
 float setVoltage =   ((float)readVDivider());
 //chg104Table((uint16_t)setVoltage);//set voltage divider
 float readVoltage = (float)getVoltage();
float newSetting = (currentVoltage/readVoltage)*setVoltage;
chg104Table((uint16_t)newSetting,0,0);//set new divider ratio
return (uint16_t)newSetting;
}

// Read 2 bytes of subclass and position offset
//subClass = subclass id
//offset = the value you want to change in case we need
// a different page
bool BQ34Z100::readFlash(uint16_t subclass, uint8_t offset)
{
    uint16_t data = 0x0000;
    // unseal device
    // byte 1
    data = 0x0414;
    WriteWord( 0x00, data );
    // byte 2
    data = 0x3672;
    WriteWord( 0x00, data );
    delay(10);
    // enable block data flash control
    WriteByte( 0x61, 0x00 ); // block data control
    delay(30);//mimic bq2300
    WriteByte( 0x3e, subclass ); // data flash class
    delay(30);//mimic bq2300
    WriteByte( 0x3f, (uint8_t)(offset / 32)); // data flash block - change this to 0x01 if offset is >31
    delay(30);//mimic bq2300
    bool changed = false;
    ReadNBytes( 0x40, flashbytes, sizeof(flashbytes) );
    delay(10);
    return false;
}

void BQ34Z100::writeTable(uint16_t subclass, uint8_t offset)
{
    uint16_t data = 0x0000;
    // unseal device
    // byte 1
    data = 0x0414;
    WriteWord( 0x00, data );
    // byte 2
    data = 0x3672;
    WriteWord( 0x00, data );
    // enable block data flash control
    WriteByte( 0x61, 0x00 ); // block data control
    delay(30);//mimic bq2300
    // access subclass
    WriteByte( 0x3e, subclass ); // data flash class
    delay(30);//mimic bq2300
    // write block offset
    WriteByte( 0x3f, (uint8_t)(offset / 32)); // data flash block - change this to 0x01 if offset is >31
    delay(30);//mimic bq2300
    bool changed = false;
    addr_flashbytes[0] = 0x40; // the address
    memcpy( &(addr_flashbytes[1]), flashbytes, sizeof(flashbytes) );
    WriteNBytes( addr_flashbytes, sizeof(addr_flashbytes) );
}

//This calcs the checksum and then writes it to the device
//This then causes the device to check it, and if correct
//It will then store the new data in flash
void BQ34Z100::checkSum(uint16_t subclass, uint8_t offset)
{
    //calc checksum
    int chkSum = 0;
    for (int i = 0; i < 32; i++)
    {
        chkSum += flashbytes[i];
    }
    int chkSumTemp = chkSum / 256;
    chkSum = chkSum - (chkSumTemp * 256);
    chkSum = 255 - chkSum;
    //We Write the whole table first
    //writeTable(subclass,offset);
    delay(35);
    //Now
    //write checksum
    WriteByte( 0x60, chkSum ); // write checksum
    delay(150);//bq neeeds time here!!!

}

void BQ34Z100::chgFlash(uint8_t index, int value)
{
    if (index > 31)
        index = index % 32;

    //  change flashbyte first
    flashbytes[index] = value;
    // write flashbyte
    WriteByte( 0x40 + index, flashbytes[index] ); // write checksum
}



void BQ34Z100::chgFlashPair(uint8_t index, int value)
{
    if (index > 31)
        index = index % 32;

    //  change flashbyte first
    flashbytes[index] = value >> 8; //high byte
    flashbytes[index + 1] = value & 0xFF; //lower byte
    // write flashbyte
    WriteByte( 0x40 + index, flashbytes[index] );
    WriteByte( 0x40 + index + 1, flashbytes[index + 1] );
}
void BQ34Z100::chgFlashQuad(uint8_t index, uint32_t value)
{
    if (index > 31)
        index = index % 32;

    //  change flashbyte first
    flashbytes[index] = value >> 24; //high byte
    flashbytes[index + 1] = value >>16; //lower byte
    flashbytes[index + 2] = value >>8; //lower byte
    flashbytes[index + 3] = value & 0xFF; //lower byte

    // write flashbyte
    WriteByte( 0x40 + index, flashbytes[index] );
    WriteByte( 0x40 + index + 1, flashbytes[index + 1] );
    WriteByte( 0x40 + index + 2, flashbytes[index + 2] );
    WriteByte( 0x40 + index + 3, flashbytes[index + 3] );
}

void BQ34Z100::chg48Table(uint16_t designCap, uint16_t designEnergy, uint16_t CellVT1T2, uint16_t CellVT2T3, uint16_t CellVT3T4, uint16_t CellVT4T5)
{
    readFlash(48, 24);
    chgFlashPair(21, designCap);
    chgFlashPair(23, designEnergy);
    chgFlashPair(28, CellVT1T2);//0x0E10 = 3600
    chgFlashPair(30, CellVT2T3);//0x0E10 = 3600
    checkSum(48, 24);
    delay(300);
    readFlash(48, 35);
    chgFlashPair(32, CellVT3T4);//0x0E10 = 3600
    chgFlashPair(34, CellVT4T5);//0x0E10 = 3600
    checkSum(48, 35);
}
void BQ34Z100::chg49Table(uint16_t UVLo, uint16_t UVLoR, uint16_t OVLo, uint16_t OVLoR)
{
    readFlash(49, 17);
    chgFlashPair(9 , UVLo);//2500

    chgFlashPair(12, UVLoR);//2900

    chgFlashPair(14, OVLo); //3900

    chgFlashPair(17, OVLoR); //3700

    checkSum(49, 17);
}
void BQ34Z100::chg64Table(uint16_t packConfigReg, uint16_t alertConfigReg, uint8_t ledChgReg, uint8_t CellsS)
{
    readFlash(64, 6);
    chgFlashPair(0, packConfigReg);
    chgFlash(4, ledChgReg);
    chgFlashPair(5, alertConfigReg);
    chgFlash(7, CellsS); //cell count
    checkSum(64, 6);
}

void BQ34Z100::chg68Table(uint16_t MinCellVoltage)
{
    readFlash(68, 0);
    chgFlashPair(0, MinCellVoltage);
    checkSum(68, 0);
}
void BQ34Z100::chg80Table(uint16_t CellTermVoltage)
{
    readFlash(80, 67);
    chgFlashPair(67, CellTermVoltage);//0A28
    checkSum(80, 67);
}
void BQ34Z100::chg82Table(uint16_t QMaxCell, uint16_t CellVAtMaxChargeTerm)
{
    readFlash(82, 5);
    chgFlashPair(0, QMaxCell);//0x4B00
    chgFlashPair(5, CellVAtMaxChargeTerm);//0x0E10
    checkSum(82, 5);
}
void BQ34Z100::chg83Table(uint16_t cellChemistry)
{
    readFlash(83, 1);
    chgFlashPair(0, cellChemistry); //LiFepo4 = 0401
    checkSum(83, 1);
}
//If vDiv is <500 dont change vdiv
//if CCG && CCD ==0 then dont change them
void BQ34Z100::chg104Table(uint16_t Vdivider,float CCGain,float CCDelta)
{
    if(Vdivider>32768)
        return;
    
        
    readFlash(0x68, 15);
    delay(30);
    if(Vdivider>500)
    chgFlashPair(14, Vdivider);
    if(!(CCGain==0 && CCDelta==0))
    {
        float GainDF = 4.768/CCGain;
        float DeltaDF = 5677445/CCDelta;
        chgFlashQuad(0,floatToXemics(GainDF));
        chgFlashQuad(4,floatToXemics(DeltaDF));
    }
    checkSum(0x68, 15);

}
uint16_t BQ34Z100::readVDivider()
{
readFlash(0x68, 15);
    uint16_t val = (((uint16_t)flashbytes[14]) <<8) | flashbytes[15];
    return val;

}
uint32_t BQ34Z100::floatToXemics(float X)
{
	int iByte1, iByte2, iByte3, iByte4, iExp;
            bool bNegative = false;
            float fMantissa;
            // Don't blow up with logs of zero
            if (X == 0) X = 0.00001F;
            if (X < 0)
            {
                bNegative = true;
                X = -X;
            }
            // find the correct exponent
            iExp = (int)((log(X) / log(2)) + 1);// remember - log of any base is ln(x)/ln(base)

            // MS byte is the exponent + 0x80
            iByte1 = iExp + 128;
           
            // Divide input by this exponent to get mantissa
            fMantissa = X / (pow(2, iExp));
           
            // Scale it up
            fMantissa = fMantissa / (pow(2, -24));
           
            // Split the mantissa into 3 bytes
            iByte2 = (int)(fMantissa / (pow(2, 16)));
            
            iByte3 = (int)((fMantissa - (iByte2 * (pow(2, 16)))) / (pow(2, 8)));
           
            iByte4 = (int)(fMantissa - (iByte2 * (pow(2, 16))) - (iByte3 * (pow(2, 8))));
           
            // subtract the sign bit if number is positive
            if (bNegative == false)
            {
                iByte2 = iByte2 & 0x7F;
            }
            return (uint32_t)((uint32_t)iByte1 << 24 | (uint32_t)iByte2 << 16 | (uint32_t)iByte3 << 8 | (uint32_t)iByte4);
}
float BQ34Z100::XemicsTofloat(uint32_t X)
{
	 bool bIsPositive = false;
            float fExponent, fResult;
            uint8_t vMSByte = (uint8_t)(X >> 24);
            uint8_t vMidHiByte = (uint8_t)(X >> 16);
            uint8_t vMidLoByte = (uint8_t)(X >> 8);
            uint8_t vLSByte = (uint8_t)X;
            // Get the sign, its in the 0x00 80 00 00 bit
            if ((vMidHiByte & 128) == 0)
            { bIsPositive = true; }

            // Get the exponent, it's 2^(MSbyte - 0x80)
            fExponent = pow(2, (vMSByte - 128));
            // Or in 0x80 to the MidHiByte
            vMidHiByte = (uint8_t)(vMidHiByte | 128);
            // get value out of midhi byte
            fResult = (vMidHiByte) * 65536;
            // add in midlow byte
            fResult = fResult + (vMidLoByte * 256);
            // add in LS byte
            fResult = fResult + vLSByte;
            // multiply by 2^-24 to get the actual fraction
            fResult = fResult * pow(2, -24);
            // multiply fraction by the ‘exponent’ part
            fResult = fResult * fExponent;
            // Make negative if necessary
            if (bIsPositive)
                return fResult;
            else
                return -fResult;
}

float BQ34Z100::readCurrentShunt()
{
    readFlash(0x68, 15);
    delay(30);
    uint32_t curentGain = ((uint32_t)flashbytes[0])<<24 | ((uint32_t)flashbytes[1])<<16|((uint32_t)flashbytes[2])<<8|(uint32_t)flashbytes[3];
    return (float) (4.768/XemicsTofloat(curentGain));
    
}
