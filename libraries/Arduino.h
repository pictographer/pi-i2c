#pragma once
#include <cstdint>
#include <wiringPi.h>
#include "CSocket.h"

#define F(X) X

#define HIGH 1
#define LOW 0

#define PROGMEM

#define A0 0
#define A1 1
#define A2 2
#define A3 3
#define A4 4
#define A5 5
#define A6 6
#define A7 7
#define A8 8
#define A9 9

long map(long x, long in_min, long in_max, long out_min, long out_max);


