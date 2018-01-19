#pragma once

#include <inttypes.h>
#include <Arduino.h>
#include <I2C.h>
#include <math.h>  

namespace bq34z100
{
        //Unshifted 7-bit I2C address for device
        const uint8_t BQ34Z100_ADDRESS = 0x55; // b1010101

	class BQ34Z100
	{
	private:

	    uint32_t Read( uint8_t address, uint8_t length);
            //Private member functions
            int32_t ReadByte( uint8_t addressIn, uint8_t& dataOut );
            int32_t ReadNBytes( uint8_t addressIn, uint8_t* dataOut, uint8_t byteCountIn );
            int32_t WriteByte( uint8_t addressIn, uint8_t dataIn );
            int32_t WriteWord( uint8_t addressIn, uint16_t dataIn );
            int32_t WriteNBytes( uint8_t *dataOut, uint8_t numberBytesOut ); 

            //Private member attributes
            uint8_t m_i2cAddress;
            I2C* m_pI2C;

	public:
	    BQ34Z100( I2C* i2cInterfaceIn );
	    void begin();
	    int writeConfig();//should only be done once
	    uint8_t getSOC();//gets the current state of charge %
	    int getTemp();//returns the temperature in C
	    int getVoltage();//returns the battery voltage
	    int getCapacity();//returns the current battery capacity
	    int getCurrent();//returns the current flowing atm
	    int getStatus();//returns the flags
	    int getRemaining();
	    int getFlags();
	    int readControl(uint8_t,uint8_t);
	    int readInstantCurrent();
	    bool readFlash(uint16_t , uint8_t );
	    int enableIT();
	    int enableCal();
	    int exitCal();
	    int enterCal();
	    void reset();
	    void checkSum(uint16_t , uint8_t);
	    uint16_t CalibrateVoltageDivider(uint16_t);//used to calibrate the voltage divider
	    void chgFlash(uint8_t, int);
	    void chgFlashPair(uint8_t, int);
	    void chg104Table(uint16_t Vdivider,float CCGain,float CCDelta);
	    void chg48Table(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);
	    void chg49Table(uint16_t, uint16_t, uint16_t, uint16_t);
	    void chg64Table(uint16_t, uint16_t, uint8_t, uint8_t);
	    void chg83Table(uint16_t);
	    void chg68Table(uint16_t);
	    void chg80Table(uint16_t);
	    void chg82Table(uint16_t, uint16_t);
	    void writeTable(uint16_t , uint8_t );
	    uint16_t readVDivider();//reads the currently set voltage divider
	    uint32_t floatToXemics(float f);
	    float XemicsTofloat(uint32_t inVal);
	    int setup(uint8_t BatteryChemistry,uint8_t SeriesCells,uint16_t CellCapacity,uint16_t PackCurrentVoltage,uint16_t current);
	    void chgFlashQuad(uint8_t index, uint32_t value);
	    void CalibrateCurrentShunt(int16_t current);
	    float readCurrentShunt();
	};
}
