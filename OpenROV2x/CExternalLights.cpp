#include "SysConfig.h"
#if(HAS_EXT_LIGHTS )

// Includes
#include <signal.h>
#include <dirent.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <I2C.h>

#include "CExternalLights.h"
#include "NCommManager.h"
#include "CMuxes.h"

extern CMuxes g_SystemMuxes;
extern I2C I2C0;

// delay to OFF time
#define OFF_TIME( value ) (pwm_map[value])
// delay to ON time
#define ON_TIME( value )  (0)

namespace
{
	// 1% per 10ms
	const float kPowerDelta = 0.01f;

	inline uint32_t PercentToAnalog( float x )
	{	
              return static_cast<uint32_t> (x*MAX_ALLOWED_POWER);
	}
}

CExternalLights::CExternalLights( uint32_t pinIn )
	: m_pin( pinIn, CPin::kAnalog, CPin::kOutput )
{
        m_targetPower = 0.0f;
        m_currentPower = 0.0f;

        m_targetPhoto = 0;
        m_targetLight = 0;
        m_lastPower_an = 0;
        m_targetPower_an = 0;
        m_currentPower_an = 0;

        m_streamer1PID = -1;
        m_streamer2PID = -1;

#ifdef OLD_BOARD
        m_led_pwm = new pca9685::PCA9685( &I2C0, pca9685::PCA9685_ADDRESS_40 );
#else
        m_led_pwm = new pca9685::PCA9685( &I2C0, pca9685::PCA9685_ADDRESS_73 );
#endif
}

int CExternalLights::getPID( uint32_t camera )
{
    int pid = -1, count = 0;
    if (camera > 2) {
       return(pid);
    }
    
    const char *comp = "mjpg_streamer";
    const char *directory = "/proc";
    size_t taskNameSize = 4096, size = 0;
    char *taskName = (char *)calloc(1, taskNameSize);


    DIR *dir = opendir(directory);
    if (dir) {
       struct dirent *de = NULL;
       while ((de = readdir(dir)) != 0) {
            if ((strcmp(de->d_name, ".") == 0) || (strcmp(de->d_name, "..") == 0))
               continue;

            pid = -1;
            int res = sscanf(de->d_name, "%d", &pid);
            if (res == 1) {
                char cmdline_file[1024] = {0};
                sprintf(cmdline_file, "%s/%d/cmdline", directory, pid);
                FILE *cmdline = fopen(cmdline_file, "r");
                if ((cmdline) && (getline(&taskName, &taskNameSize, cmdline)  > 0)) {
                   if (strstr(taskName, comp) != 0) {
                      if (++count == camera) {
                        fclose(cmdline);
                        closedir(dir);
                      	return(pid);
                      }
                   }
                }
                if (cmdline) fclose(cmdline);
            }
       }
    }
    closedir(dir);
    free(taskName);
    return(-1);
}

void CExternalLights::getIP( char addressBuffer[INET_ADDRSTRLEN] )
{
    struct ifaddrs *ifAddrStruct = NULL, *ifa = NULL;
    void *tmpAddrPtr = NULL;

    getifaddrs(&ifAddrStruct);
    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        // IPv4
        if (ifa->ifa_addr->sa_family == AF_INET) {
            // wlan0
            if (strcmp(ifa->ifa_name,"wlan0") == 0) {
               tmpAddrPtr = &((struct sockaddr_in *) ifa->ifa_addr)->sin_addr;
               inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN); 
               break;
            }
        }
    }
    if (ifAddrStruct != NULL) {
        freeifaddrs(ifAddrStruct);
    }
}

void CExternalLights::snapPhoto( uint32_t camera )
{
   char command[512] = {'\0'};
   // camera 1 and 2 on on RPiA
   // http port 9091 and 9092
   // camera 3 and 4 on on RPiB
   // http port 9091 and 9092
   // so we need to execute the command line to get the snapshot
   uint32_t port = 9091 + (camera % 2 != 0 ?  0 : 1);
   if (camera <= 2) {
       sprintf(command,"wget http://%s:%d/?action=snapshot -O /home/pi/photos/photo.%d.%u.jpg", m_address_A, port, camera, time(NULL) );
   } else {
       sprintf(command,"wget http://%s:%d/?action=snapshot -O /home/pi/photos/photo.%d.%u.jpg", m_address_B, port, camera, time(NULL) );
   }
   // execute the command
   system( command ); 
}

void CExternalLights::streamerControl(uint32_t camera )
{
   // active camera is 'camera'
   if (strchr( m_hostname, 'A' ) != NULL) {
      // PI A
      if ((camera == 0) || (camera > 2)) {
        if (m_streamer1PID != -1) kill(m_streamer1PID, SIGSTOP);
        if (m_streamer2PID != -1) kill(m_streamer2PID, SIGSTOP);
        // and pass on message to RPiB
      } else {
         if (camera == 1) {
            if (m_streamer1PID != -1) kill(m_streamer1PID, SIGCONT);
            if (m_streamer2PID != -1) kill(m_streamer2PID, SIGSTOP);
         } else {
            if (m_streamer1PID != -1) kill(m_streamer1PID, SIGSTOP);
            if (m_streamer2PID != -1) kill(m_streamer2PID, SIGCONT);
         }
      }
   } else {
     // PI B
     if (camera <=2 ) {
        if (m_streamer1PID != -1) kill(m_streamer1PID, SIGSTOP);
        if (m_streamer2PID != -1) kill(m_streamer2PID, SIGSTOP);
     } else {
        if (camera == 3) {
           if (m_streamer1PID != -1) kill(m_streamer1PID, SIGCONT);
           if (m_streamer2PID != -1) kill(m_streamer2PID, SIGSTOP);
        } else {
           if (m_streamer1PID != -1) kill(m_streamer1PID, SIGSTOP);
           if (m_streamer2PID != -1) kill(m_streamer2PID, SIGCONT);
        }
     }
   }


}

void CExternalLights::Initialize()
{
    getIP( m_address );
    gethostname(m_hostname, HOST_NAME_MAX);
    // parse out the last triple from the ip address
    char *p = strrchr( m_address, '.' );
    // grab those digits and turn them into a number
    char tmp[16];
    strcpy( tmp, p+1 );
    int number = atoi( tmp );

    // OK. Figure out if I am A or B
    if (strchr( m_hostname, 'A' ) == NULL) {
       // m_address has IP address of B
       strcpy( m_address_B, m_address );
       number = number - 1;
       *p = '\0';
       // now build the new ip address
       sprintf( m_address_A, "%s.%d", m_address, number );
    } else {
       // m_address has IP address of A
       strcpy( m_address_A, m_address );
       number = number + 1;
       *p = '\0';
       // now build the new ip address
       sprintf( m_address_B, "%s.%d", m_address, number );
    }

    // enable power to LEDs
#ifdef OLD_BOARD
    g_SystemMuxes.WriteExtendedGPIO(BAL_LEDS_EN, LOW);
    // get the PWM ready
    g_SystemMuxes.SetPath(SCL_PWM);
#else
    g_SystemMuxes.WriteExtendedGPIO(BAL_EN, LOW);
    g_SystemMuxes.SetPath(SCL_NONE);
#endif
    // top
    m_led_pwm->DigitalWriteLow(pca9685::LED_10);
    // front
    m_led_pwm->DigitalWriteLow(pca9685::LED_11);
    // bottom
    m_led_pwm->DigitalWriteLow(pca9685::LED_12);
    // side
    m_led_pwm->DigitalWriteLow(pca9685::LED_13);
    m_led_pwm->SetPreScale(0x03);
    m_led_pwm->UnSleep();

    // Reset timers
    m_controlTimer.Reset();
    m_telemetryTimer.Reset();
}

void CExternalLights::Update( CCommand& commandIn )
{
    // m_streamer1PID = getPID( 1 );
    // m_streamer2PID = getPID( 2 );

	// Check for messages
	if( NCommManager::m_isCommandAvailable )
	{
		// Handle messages
		if( commandIn.Equals( "elights_tpow" ) )
		{
                        // printf("lights: raw %d\n", commandIn.m_arguments[1]);
			// Update the target position
			m_targetPower = orutil::Decode1K( commandIn.m_arguments[1] );
                        // printf("lights: %f\n", m_targetPower);

			// TODO: Ideally this unit would have the ability to autonomously set its own target and ack receipt with a separate mechanism
			// Acknowledge target position
                        delay(50);
			Serial.print( F( "elights_tpow:" ) );
			Serial.print( commandIn.m_arguments[1] );
			Serial.print( ';' );
	                Serial.print( F( "ENDUPDATE:1;" ) );

			// Pass through linearization function
			m_targetPower_an = PercentToAnalog( m_targetPower );

			// Apply ceiling
			if( m_targetPower_an > MAX_ALLOWED_POWER )
			{
				m_targetPower_an = MAX_ALLOWED_POWER;
			}

			// Directly move to target power
			m_currentPower 		= m_targetPower;
			m_currentPower_an 	= m_targetPower_an;

                        SetLight();
			// Emit current power
			Serial.print( F( "elights_pow:" ) );
			Serial.print( orutil::Encode1K( m_targetPower ) );
			Serial.print( ';' );
	                Serial.print( F( "ENDUPDATE:1;" ) );
		} else {
		        if( commandIn.Equals( "elights_select" ) )
		        {
                            // printf("lights: select %d\n", commandIn.m_arguments[1]);
			    // Update the target position
                            // 0: all lights
                            // 2: camera 1 Front
                            // 1: camera 2 Side
                            // 4: camera 3 Top
                            // 3: camera 4 Bottom
			    m_targetLight = commandIn.m_arguments[1];
			    Serial.print( F( "elights_select:" ) );
			    Serial.print( m_targetLight );
			    Serial.print( ';' );
	                    Serial.print( F( "ENDUPDATE:1;" ) );
                            // streamerControl(m_targetLight);
                            // force all LEDs on by making
                            // m_targetLight equal to zero
                            m_targetLight = 0;
                            SetLight();
		        } else {
		            if( commandIn.Equals( "ephoto_select" ) )
		            {
                                // printf("photo: select %d\n", commandIn.m_arguments[1]);
			        // Update the target position
                                // 0: all lights
                                // 2: camera 1 Front
                                // 1: camera 2 Side
                                // 4: camera 3 Top
                                // 3: camera 4 Bottom
			        m_targetPhoto = commandIn.m_arguments[1];
			        Serial.print( F( "ephoto_select:" ) );
			        Serial.print( m_targetPhoto );
			        Serial.print( ';' );
	                        Serial.print( F( "ENDUPDATE:1;" ) );
                                snapPhoto( m_targetPhoto );
		            }
                        }
                }
	}
}

void CExternalLights::SetLight( void ) 
{
#ifdef OLD_BOARD
        g_SystemMuxes.SetPath(SCL_PWM);
#else
        g_SystemMuxes.SetPath(SCL_NONE);
#endif
        if (m_currentPower_an == 0) {
            // bottom
            if ((m_targetLight == 0) || (m_targetLight == 3))
                 m_led_pwm->DigitalWriteLow(pca9685::LED_10);
            // side
            if ((m_targetLight == 0) || (m_targetLight == 1))
                 m_led_pwm->DigitalWriteLow(pca9685::LED_11);
            // top
            if ((m_targetLight == 0) || (m_targetLight == 4))
                 m_led_pwm->DigitalWriteLow(pca9685::LED_12);
            // front
            if ((m_targetLight == 0) || (m_targetLight == 2))
                 m_led_pwm->DigitalWriteLow(pca9685::LED_13);
        } else {
            // range 0-255 m_targetPower
            // bottom
            if ((m_targetLight == 0) || (m_targetLight == 3)) {
                 m_led_pwm->DigitalWrite(pca9685::LED_10, ON_TIME(m_currentPower_an), OFF_TIME(m_currentPower_an));
                 if (m_targetLight != 0) {
                     m_led_pwm->DigitalWriteLow(pca9685::LED_11);
                     m_led_pwm->DigitalWriteLow(pca9685::LED_12);
                     m_led_pwm->DigitalWriteLow(pca9685::LED_13);
                 }
            }
            // side
            if ((m_targetLight == 0) || (m_targetLight == 1)) {
                 m_led_pwm->DigitalWrite(pca9685::LED_11, ON_TIME(m_currentPower_an), OFF_TIME(m_currentPower_an));
                 if (m_targetLight != 0) {
                     m_led_pwm->DigitalWriteLow(pca9685::LED_10);
                     m_led_pwm->DigitalWriteLow(pca9685::LED_12);
                     m_led_pwm->DigitalWriteLow(pca9685::LED_13);
                 }
            }
            // top
            if ((m_targetLight == 0) || (m_targetLight == 4)) {
                 m_led_pwm->DigitalWrite(pca9685::LED_12, ON_TIME(m_currentPower_an), OFF_TIME(m_currentPower_an));
                 if (m_targetLight != 0) {
                     m_led_pwm->DigitalWriteLow(pca9685::LED_10);
                     m_led_pwm->DigitalWriteLow(pca9685::LED_11);
                     m_led_pwm->DigitalWriteLow(pca9685::LED_13);
                 }
            }
            // front
            if ((m_targetLight == 0) || (m_targetLight == 2)) {
                 m_led_pwm->DigitalWrite(pca9685::LED_13, ON_TIME(m_currentPower_an), OFF_TIME(m_currentPower_an));
                 if (m_targetLight != 0) {
                     m_led_pwm->DigitalWriteLow(pca9685::LED_10);
                     m_led_pwm->DigitalWriteLow(pca9685::LED_11);
                     m_led_pwm->DigitalWriteLow(pca9685::LED_12);
                 }
            }
        }
        delay(50);
}
#endif
