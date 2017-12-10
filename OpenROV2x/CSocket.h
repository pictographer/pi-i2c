/*
	Socket server with an interface similar to the Arduino Serial class
*/

#pragma once

#include <stdio.h>
#include <string.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <errno.h>
#include <cstdint>

const size_t CSocket_print_max = 80;

typedef enum { DEC=10, HEX=16 } CSocket_print_t;

class CSocket {
  public:
   CSocket();
   int available();
   void begin(int ignore_baud_rate=0);
   int read();

   int print(long, CSocket_print_t format=DEC);
   int print(int, CSocket_print_t format=DEC);
   int print(uint32_t, CSocket_print_t format=DEC);
   int print(short unsigned int, CSocket_print_t format=DEC);
   int print(uint8_t, CSocket_print_t format=DEC);
   int print(char c);
   int print(const char* s);
   int print(float f);
   int print(bool b);

   int println(long, CSocket_print_t format=DEC);
   int println(int, CSocket_print_t format=DEC);
   int println(char c);
   int println(uint32_t u, CSocket_print_t format);
   int println(uint8_t, CSocket_print_t format=DEC);
   int println(const char* s="");
  protected:
   int sockfd;
   int clientfd;
   struct sockaddr_in server_in, client_in;

   uint8_t buf[2000];
   size_t buf_len;
   size_t buf_start;
};

extern CSocket Serial;
