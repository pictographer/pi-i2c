/*
        Socket server with an interface similar to the Arduino Serial class
*/

#include "CSocket.h"
#include <limits>
#include <stdexcept>
#include <cstddef>
#include <inttypes.h>
#include <cfloat>
#include <netinet/tcp.h>

CSocket::CSocket() : buf_len(0), buf_start(0) {
   // begin(); // Might be a bad idea to call begin here, but where?
   socket_ready = 0;
}

int CSocket::available() {
   if (buf_len == 0) {
      buf_len = recv(clientfd, buf, 4096, 0);

      buf[buf_len] = 0;

      buf_start = 0;
      if (buf_len == 0) {
         printf("Socket client disconnected.\n");
         fflush(stdout);
      } else if (buf_len == -1) {
         perror("Socket recv failed. Error");
      } else if (buf_len < 0) {
         buf_len = 0;
      }
   }
   return buf_len;
}

void CSocket::end() {
   puts("Closing socket connection");
   close(clientfd);
   close(sockfd);
   buf_len = 0;
   buf_start = 0;
   socket_ready = 0;
}

// Start the socket listener.
void CSocket::begin(int) {
   if (socket_ready == 1) return;

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
      puts("listen error");
      perror("Unable to listen on socket. Error");
   }

   size_t len = sizeof(struct sockaddr_in);

   puts("Waiting for incoming connections...");

   clientfd = accept(sockfd, (struct sockaddr *)&client_in, (socklen_t*)&len);
   if (clientfd < 0) {
      perror("Unable to accept socket connection. Error");
   }
   printf("Socket server connection accepted.\n");
   socket_ready = 1;
}

// If data is available in the buffer, update start, update length, and return
// the character previously at start. Otherwise, await data from the socket and
// return a character when available. Print a message if the client goes away.
// Report errors.
//
// TODO: I haven't made a way of distinguishing no client from 0xFF
// appearing in the input stream.
int CSocket::read() {
   int result = -1;
   if (available()) {
      result = buf[buf_start];
      --buf_len;
      ++buf_start;
   }
   return result;
}

int CSocket::print(long n, CSocket_print_t format) {
   // sprintf a long into the buffer
   // if there is space available in the buffer.
   char long_buf[std::numeric_limits<long>::digits10 + 1];
   snprintf(long_buf, sizeof long_buf, format == DEC ? "%ld" : "%lx", n);
   int result = write(clientfd, long_buf, strlen(long_buf));
   if (result < 0) {
      perror("Unable to print long integer. Error");
   }
   return result;
}

int CSocket::print(int n, CSocket_print_t format) {
   return print(long(n), format);
}

//int CSocket::print(int32_t u, CSocket_print_t format) {
//   return print(long(u), format);
//}

int CSocket::print(uint32_t u, CSocket_print_t format) {
   char uint32_buf[std::numeric_limits<uint32_t>::digits10 + 1];
   snprintf(uint32_buf, sizeof uint32_buf,
            format == DEC ? "%" PRIu32 : "%" PRIx32, u);
   int result = write(clientfd, uint32_buf, strlen(uint32_buf));
   if (result < 0) {
      perror("Unable to print uint32_t integer. Error");
   }
   return result;
}

int print(short unsigned int u, CSocket_print_t format=DEC) {
   return print(uint32_t(u), format);
}

int CSocket::print(uint8_t u, CSocket_print_t format) {
   return print(uint32_t(u), format);
}

int CSocket::print(char c) {
   ssize_t result = -1;
   result = write(clientfd, &c, 1);
   return( result );
}

int CSocket::print(const char* s) {
   size_t len = strlen(s);
   ssize_t result = -1;
   if (len <= CSocket_print_max) {
      result = write(clientfd, s, len);
      if (result < 0) {
         perror("Socket write failed printing string. Error");
      }
   }
   return result;
}

// If we need bug-for-bug compatibility go here:
// https://github.com/arduino/Arduino/blob/master/hardware/arduino/avr/cores/arduino/Print.cpp
int CSocket::print(float f) {
   // Found the answer for doubles. No time to optimize.
   char float_buf[3 + DBL_MANT_DIG - DBL_MIN_EXP + 1];
   snprintf(float_buf, sizeof float_buf, "%f", f);
   int result = write(clientfd, float_buf, strlen(float_buf));
   if (result < 0) {
      perror("Unable to print float. Error");
   }
   return result;
}

int CSocket::print(bool b) {
   return write(clientfd, b ? "1" : "0", 1);
}


int CSocket::println(long n, CSocket_print_t format) {
   return print(n, format) + print('\n');
}

int CSocket::println(int n, CSocket_print_t format) {
   return print(n, format) + print('\n');
}

int CSocket::println(char c) {
   print(c);
   print('\n');
   return 2;
}

int CSocket::println(uint32_t u, CSocket_print_t format) {
   return print(u, format) + print('\n');
}

int CSocket::println(uint8_t u, CSocket_print_t format) {
   return println(uint32_t(u), format);
}

int CSocket::println(const char* s) {
   ssize_t result = print(s);
   if (0 <= result) {
      print('\n');
   }
   return result + 1;
}
