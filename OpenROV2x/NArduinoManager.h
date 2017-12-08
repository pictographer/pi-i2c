#pragma once

// Includes
#include <Arduino.h>
#if defined( ARDUINO_ARCH_AVR )
#include <avr/wdt.h>
#endif

namespace NArduinoManager
{
	// Variables
	extern volatile uint8_t	m_wdtResetInfo;

	// Methods
	extern void Initialize();

	extern void EnableWatchdogTimer();
	extern void DisableWatchdogTimer();

#if defined( ARDUINO_ARCH_AVR )
	ISR( WDT_vect );
#else
        //XXX Replace this with a suitable watchdog timer someday.
	#define ISR(x)
#endif
}

