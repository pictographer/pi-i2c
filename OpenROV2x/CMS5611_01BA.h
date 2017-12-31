#pragma once

// Includes
#include <MS5611_01BA.h>
#include <orutil.h>
#include "CModule.h"

class CMS5611_01BA : public CModule
{
public:
	CMS5611_01BA( I2C *i2cInterfaceIn );
	void Initialize();
	void Update( CCommand& commandIn );

private:
	// Device driver
	ms5611_01ba::MS5611_01BA m_device;

	// Timers
	orutil::CTimer m_statusCheckTimer;
	orutil::CTimer m_telemetryTimer;

	float m_depthOffset_m = 0.0f;
	uint32_t m_maxFailuresPerPeriod = 0;
	bool m_hasLock = false;
};
