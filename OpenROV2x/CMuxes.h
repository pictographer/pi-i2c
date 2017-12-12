#pragma once

// Includes
#include <PCA9547.h>
#include "CCommand.h"
#include "CModule.h"

class CMuxes : public CModule
{
public:
	CMuxes();
	void Initialize();
	void Update( CCommand &commandIn );
        void SetPath( uint32_t path );

private:
	// Device driver
	pca9547::PCA9547 *m_device0;
	pca9547::PCA9547 *m_device1;
	pca9547::PCA9547 *m_device2;

        I2C  *i2cInterface0;
        I2C  *i2cInterface1;
        I2C  *i2cInterface2;

};



