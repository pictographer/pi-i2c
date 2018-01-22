#pragma once

// Includes
#include <P89BSD012BS.h>
#include <orutil.h>
#include "CModule.h"

class CP89BSD012BS : public CModule
{
public:
	CP89BSD012BS( I2C *i2cInterfaceIn );
	void Initialize();
	void Update( CCommand& commandIn );
        uint8_t GetMaxPressureFlag( void );

private:
	// Device driver
	p89bsd012bs::P89BSD012BS m_device;

	// Timers
	orutil::CTimer m_statusCheckTimer;
	orutil::CTimer m_telemetryTimer;

	float m_depthOffset_m = 0.0f;
	uint32_t m_maxFailuresPerPeriod = 0;
	bool m_hasLock = false;
};
