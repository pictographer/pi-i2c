// Includes
#if defined( ARDUINO_ARCH_AVR )
#include <EEPROM.h>
#endif

#include "NArduinoManager.h"
#include "NVehicleManager.h"
#include "NDataManager.h"
#include "NCommManager.h"
#include "NModuleManager.h"
#include "CMuxes.h"

#if defined( WIRINGPI )
#include <wiringPi.h>
#include "CSocket.h"
CSocket Serial;
long map(long x, long in_min, long in_max, long out_min, long out_max)
{
   return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
#endif

//
// Make the high level system muxes accessible all over
//
CMuxes g_SystemMuxes;

void setup()
{

        // Initialize main subsystems
        NArduinoManager::Initialize();
        NCommManager::Initialize();
        NVehicleManager::Initialize();

        wiringPiSetup();
        g_SystemMuxes.Initialize();
        NModuleManager::Initialize();
        NDataManager::Initialize();

        // Power System Arm
#if 0
        // '9539 low -> high (1 second) -> low
        g_SystemMuxes.WriteExtendedGPIO(RLY_ARM, LOW);
        delay(1000);
        g_SystemMuxes.WriteExtendedGPIO(RLY_ARM, HIGH);
        delay(1000);
        g_SystemMuxes.WriteExtendedGPIO(RLY_ARM, LOW);
#endif

        // Set timer 5 divisor to 8 for PWM frequency of 3921.16Hz (D44, D45, D46)
        // TCCR5B = ( TCCR5B & B11111000 ) | B00000010;

        // Boot complete
        Serial.println( F( "boot:1;" ) );
}

void loop()
{
#warning Watchdog timer not implemented on this platform
        // Reset the watchdog timer
        // wdt_reset();

        // Attempt to read a current command off of the command line
        NCommManager::GetCurrentCommand();

        // Handle any config change requests
        NVehicleManager::HandleMessages( NCommManager::m_currentCommand );

        // Handle update loops for each module
        NModuleManager::HandleModuleUpdates( NCommManager::m_currentCommand );

        // Handle update loops that send data back to the beaglebone
        NDataManager::HandleOutputLoops();
}

int main(int, char**) {
   setup();
   while (1) loop();
   return 0; // not reached
}
