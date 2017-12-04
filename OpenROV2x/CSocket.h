/*
	Socket server with an interface similar to the Arduino Serial class
*/

#pragma once

#if 0

#include <stdio.h>
#include <string.h>    //strlen
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <errno.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

#else

#include<stdio.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include <errno.h>

#endif

const size_t CSocket_print_max = 80;

typedef enum { DEC, HEX } CSocket_print_t;

class CSocket {
  public:
   CSocket();
   int available();
   void begin();
   int read();
   int print(long, CSocket_print_t format=DEC);
   int print(char c);
   int print(const char* s);
   int println(long, CSocket_print_t format=DEC);
   int println(char c);
   int println(const char* s);
  protected:
   int sockfd;
   struct sockaddr_in server_in, client_in;

   uint8_t buf[2000];
   size_t buf_len;
   size_t buf_start;
};
