#pragma once

// Includes
#include <P86BSD030PA.h>
#include <orutil.h>
#include "CModule.h"

class CP86BSD030PA : public CModule
{
public:
	CP86BSD030PA( I2C *i2cInterfaceIn );
	void Initialize();
	void Update( CCommand& commandIn );

private:
	// Device driver
	p86bsd030pa::P86BSD030PA m_device;

	// Timers
	orutil::CTimer m_statusCheckTimer;
	orutil::CTimer m_telemetryTimer;

	float m_depthOffset_m = 0.0f;
	uint32_t m_maxFailuresPerPeriod = 0;
	bool m_hasLock = false;
};
