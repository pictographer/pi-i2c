#pragma once

// Includes
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <limits.h>

#include "PCA9685.h"
#include "CModule.h"
#include "CPin.h"
#include <orutil.h>

// Map to 10 discrete steps
#define MAX_ALLOWED_POWER 10

class CExternalLights : public CModule
{
public:
    CExternalLights( uint32_t pinIn );
    void Initialize();
    void Update( CCommand& commandIn );
    void SetLight( void );

private:
    orutil::CTimer      m_controlTimer;
    orutil::CTimer      m_telemetryTimer;
    pca9685::PCA9685   *m_led_pwm;
    CPin        m_pin;

    float       m_targetPower = 0.0f;
    float       m_currentPower = 0.0f;

    char        m_address[INET_ADDRSTRLEN];
    char        m_hostname[HOST_NAME_MAX];
    char        m_address_A[INET_ADDRSTRLEN];
    char        m_address_B[INET_ADDRSTRLEN];
    uint32_t    m_targetPhoto = 0;
    uint32_t    m_targetLight = 0;
    uint32_t    m_lastPower_an = 0;
    uint32_t    m_targetPower_an = 0;
    uint32_t    m_currentPower_an = 0;

    int         m_streamer1PID = -1;
    int         m_streamer2PID = -1;

    uint16_t    pwm_map[11] = { 0, 106, 148, 205, 285, 396, 550, 764, 1062, 1475, 2048 };

    void snapPhoto( uint32_t camera );
    void getIP( char addressBuffer[INET_ADDRSTRLEN] );
    int getPID( uint32_t camera );
    void streamerControl( uint32_t camera );
};

