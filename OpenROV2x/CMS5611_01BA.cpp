#include "SysConfig.h"
#if(HAS_MS5611_01BA)

#include "I2C.h"
#include "CMS5611_01BA.h"
#include "NCommManager.h"
#include "CMuxes.h"

extern CMuxes g_SystemMuxes;
extern I2C I2C0;

using namespace ms5611_01ba;

namespace
{
	constexpr uint32_t kMaxHardResets 		= 5;

	constexpr uint32_t kStatusCheckDelay_ms	= 1000;							// 1hz
	constexpr uint32_t kTelemetryDelay_ms 	= 100;							// 10hz
}

CMS5611_01BA::CMS5611_01BA( I2C *i2cInterfaceIn )
	: m_device( i2cInterfaceIn )
{
}

void CMS5611_01BA::Initialize()
{
	Serial.print( F( "ms5611_init:1;" ) );
	Serial.print( F( "ENDUPDATE:1;" ) );

	m_statusCheckTimer.Reset();
	m_telemetryTimer.Reset();

	// 25% max failure rate before 
	m_maxFailuresPerPeriod = ( kStatusCheckDelay_ms / 4 ) / m_device.GetUpdatePeriod();
}

void CMS5611_01BA::Update( CCommand& commandIn )
{
	if( m_device.IsEnabled() == false )
	{
		return;
	}

	// Check for lock
	if( m_hasLock == false )
	{
		// Attempt to get the lock
		m_hasLock = m_device.GetLock();

		if( m_hasLock == false )
		{
			// Don't allow the code to proceed
			return;
		}
	}

	// Handle health checks
	if( m_statusCheckTimer.HasElapsed( kStatusCheckDelay_ms ) )
	{
		// Check to see if the error threshold is above acceptable levels
		if( m_device.GetResultCount( EResult::RESULT_ERR_FAILED_SEQUENCE ) > m_maxFailuresPerPeriod )
		{
			Serial.print( "ms5611_HardReset:1" );
			m_device.HardReset();
	                Serial.print( F( "ENDUPDATE:1;" ) );
		}
		else
		{
			// Clear the error counter
			m_device.ClearResultCount( EResult::RESULT_ERR_FAILED_SEQUENCE );
		}
		
		// Check to see if we have surpassed our hard reset threshhold 
		if( m_device.GetResultCount( EResult::RESULT_ERR_HARD_RESET ) > kMaxHardResets )
		{
			// Free the lock
			if( m_hasLock )
			{
				m_hasLock = false;
				m_device.FreeLock();
			}

			// Permanently disable the device
			m_device.Disable();
			Serial.print( F( "ms5611_disabled:1;" ) );
	                Serial.print( F( "ENDUPDATE:1;" ) );
			return;
		}
	}
    
	// Handle commands
	if( NCommManager::m_isCommandAvailable )
	{
		// Zero the depth value
		if( commandIn.Equals( "depth_zero" ) )
		{
			// Set offset based on current value
			m_depthOffset_m = m_device.m_data.depth_m;

			// Send ack
			Serial.print( F( "depth_zero:ack;" ) );
	                Serial.print( F( "ENDUPDATE:1;" ) );
		}
		// Clear the depth offset
		else if( commandIn.Equals( "depth_clroff" ) )
		{
			// Set offset based on current value
			m_depthOffset_m = 0.0f;

			// Send ack
			Serial.print( F( "depth_clroff:ack;" ) );
	                Serial.print( F( "ENDUPDATE:1;" ) );
		}
		// Change water type
		else if( commandIn.Equals( "depth_water" ) )
		{
#ifdef DEPTH_WATER
			if( commandIn.m_arguments[1] == (uint32_t)EWaterType::FRESH )
			{
				m_device.SetWaterType( EWaterType::FRESH );

				// Ack
				Serial.print( F( "depth_water:" ) );	
				Serial.print( commandIn.m_arguments[1] ); 	
				Serial.print( ';' );
	                        Serial.print( F( "ENDUPDATE:1;" ) );
			}
			else if( commandIn.m_arguments[1] == (uint32_t)EWaterType::SALT )
			{
				m_device.SetWaterType( EWaterType::SALT );

				// Ack
				Serial.print( F( "depth_water:" ) );	
				Serial.print( commandIn.m_arguments[1] ); 	
				Serial.print( ';' );
	                        Serial.print( F( "ENDUPDATE:1;" ) );
			}
#endif
		}
	}

        // set path to enable access to sensor
        g_SystemMuxes.SetPath(SCL_PXDCR);
	// Tick device state machine
	m_device.Tick();

	// Emit telemetry
	if( m_telemetryTimer.HasElapsed( kTelemetryDelay_ms ) )
	{
		if( m_device.m_data.SampleAvailable() )
		{
			// Report results
			Serial.print( F( "depth_t:" ) );	Serial.print( orutil::Encode1K( m_device.m_data.temperature_c ) ); 	Serial.print( ';' );
			Serial.print( F( "depth_p:" ) );	Serial.print( orutil::Encode1K( m_device.m_data.pressure_mbar ) ); 	Serial.print( ';' );
			Serial.print( F( "depth_d:" ) );	Serial.print( orutil::Encode1K( m_device.m_data.depth_m ) ); 		Serial.print( ';' );
	                Serial.print( F( "ENDUPDATE:1;" ) );
		}
	}
}

#endif
