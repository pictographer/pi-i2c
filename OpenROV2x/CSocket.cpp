/*
        Socket server with an interface similar to the Arduino Serial class
*/

#include "CSocket.h"
#include <limits>
#include <stdexcept>

#if 0
#include <cstdio>
#include <sys/socket.h>

#include <stdio.h>
#include <string.h>    //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <errno.h>
#include <cstddef>
#endif

CSocket::CSocket() : buf_len(0), buf_start(0) {}

int CSocket::available() {
   return buf_len;
}

// Start the socket listener.
void CSocket::begin() {
   const int default_protocol = 0;
   sockfd = socket(AF_INET, SOCK_STREAM, default_protocol);
   if (sockfd == -1) {
      perror("Socket creation failed. Error");
   }

   struct sockaddr_in server;

   //Prepare the sockaddr_in structure
   server.sin_family = AF_INET;
   server.sin_addr.s_addr = INADDR_ANY;
   server.sin_port = htons(5432);

   //Bind
   if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
   {
      //print the error message
      perror("bind failed. Error");
      throw;
   }
   puts("bind done");

   server_in = server;

   if (listen(sockfd, 3) < 0) {
      perror("Unable to listen on socket. Error");
   }

   size_t len = sizeof(struct sockaddr_in);

   clientfd = accept(sockfd, (struct sockaddr *)&client_in, (socklen_t*)&len);
   if (clientfd < 0) {
      perror("Unable to accept socket connection. Error");
   }
   if (clientfd != sockfd) {
      printf("That's interesting. client_sock != sockfd\n");
   }
   printf("Socket server awaiting connections.\n");
}

// If data is available in the buffer, update start, update length, and return
// the character previously at start. Otherwise, await data from the socket and
// return a character when available. Generate an error if the client goes away.
//
// TODO: I haven't made a way of distinguishing no client from 0xFF
// appearing in the input stream.
int CSocket::read() {
   int result = -1;
   if (0 < buf_len) {
      result = buf[buf_start];
      --buf_len;
      ++buf_start;
   } else {
      buf_len = recv(clientfd, buf, 2000, 0);
      buf_start = 0;
      if (0 < buf_len) {
         result = buf[0];
         buf_start = 1;
      } else if (buf_len == 0) {
         printf("Socket client disconnected.\n");
         fflush(stdout);
      } else if (buf_len == -1) {
         perror("Socket recv failed. Error");
      }
   }
   return result;
}

int CSocket::print(long n, CSocket_print_t format) {
   // sprintf a long into the buffer
   // if there is space available in the buffer.
   char long_buf[std::numeric_limits<long>::digits10 + 1];
   snprintf(long_buf, sizeof long_buf, format == DEC ? "%ld" : "%lx", n);
   int result = write(sockfd, long_buf, sizeof long_buf);
   if (result < 0) {
      perror("Unable to print long integer. Error");
   }
}

int CSocket::print(char c) {
   return write(sockfd, &c, 1);
}

int CSocket::print(const char* s) {
   size_t len = strlen(s);
   ssize_t result = -1;
   if (len <= CSocket_print_max) {
      result = write(sockfd, s, len);
      if (result < 0) {
         perror("Socket write failed printing string. Error");
      }
   }
   return result;
}

int CSocket::println(long n, CSocket_print_t format) {
   return print(n) + print('\n');
}

int CSocket::println(char c) {
   print(c);
   print('\n');
   return 2;
}

int CSocket::println(const char* s) {
   ssize_t result = print(s);
   if (0 <= result) {
      print('\n');
   }
   return result + 1;
}
