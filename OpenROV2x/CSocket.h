/*
	Socket server with an interface similar to the Arduino Serial class
*/

#pragma once

typedef enum { DEC, HEX } CSocket_print_t;

class CSocket {
public:
  bool available();
  void begin();
  int read();
  int write();
  int print(long,format=DEC);
  int print(char*);
  int print(char);
  int println();
};
