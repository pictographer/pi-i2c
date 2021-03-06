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
// The global restart system flag
uint8_t restart;

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
        // '9539 low -> high (1 second) -> low
        g_SystemMuxes.WriteExtendedGPIO(RLY_ARM, LOW);
        delay(1000);
        g_SystemMuxes.WriteExtendedGPIO(RLY_ARM, HIGH);
        delay(1000);
        g_SystemMuxes.WriteExtendedGPIO(RLY_ARM, LOW);

        // Set timer 5 divisor to 8 for PWM frequency of 3921.16Hz (D44, D45, D46)
        // TCCR5B = ( TCCR5B & B11111000 ) | B00000010;

        // Boot complete
        Serial.print( F( "boot:1;" ) );
        Serial.print( F( "ENDUPDATE:1;" ) );
        // we have initiated restart
        restart = 0;
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
   // restart is a global variable modified by a command from the client
   // to force a system restart. This will be enabled when one PI is
   // commanded to take over the ROV operation
   restart = 1;
   while (restart == 1) {
       // restart is set to 0 in setup()
       setup();
       // the takeover() command will set restart to 1
       while (restart == 0) loop();
   }
   return 0; // not reached
}
