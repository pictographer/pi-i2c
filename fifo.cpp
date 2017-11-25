#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

// This application is a proof of principle that we can replace the Arduino
// driving I2C devices with code running on the Raspberry Pi directly.
// The serial channel is replaced with a pair of named pipes. The rx channel
// receives high-level commands from the controlling software (GUI, etc.).
// The tx channel replaces the serial output of the Arduino.


// We need a way to tell where we get I2C stuff, alas wiringPiI2C.h doesn't 
// define any preprocessor symbols.
#define _WIRINGPII2C_H_
// Might not need to include wiringPi.h because all we need is in
// wiringPiI2C.h.
#include <wiringPi.h>
#include <wiringPiI2C.h>

#include <BNO055.h>

// This pipe is read for control (GUI) or supervisory commands. 
#define RX_FIFO "rx"

// This pipe is written with sensor output and other low-level status.
#define TX_FIFO "tx"

// File descriptors
int rx_fd = -1;
int tx_fd = -1;
int i2c_fd = -1;

int create_rx_fifo() {
   return mkfifo(RX_FIFO, 0777);
}

int create_tx_fifo() {
   return mkfifo(TX_FIFO, 0777);
}

int open_rx_fifo() {
   return open(RX_FIFO, O_RDONLY);
}

int open_tx_fifo() {
   return open(TX_FIFO, O_WRONLY);
}

void close_rx_fifo() { close(rx_fd); }
void close_tx_fifo() { close(tx_fd); }

// Currently unused.
void delete_rx_fifo() { unlink(RX_FIFO); }
void delete_tx_fifo() { unlink(TX_FIFO); }

void receive_command() {
   // Read a command from the rx fifo.
   // Parse it.
   // Add it to the command ring buffer.
   // Preserve previous overrun behavior and acknowledge that the oldest
   //    command will arbitrarily be overwritten if we fall behind.
}

void parse_command() {
    // Split the command into name and numbers.
}

void dispatch_command() {
   // Give every command object a chance to react to the command
}

void report_command_result() {}

int main(int argc, char **argv)
{
   int result;

   i2c_fd = wiringPiI2CSetup(0x20); // XXX bogus I2C address for debugging
   printf("wiringPiI2CSetup() ==> %d\n", i2c_fd);

   result = wiringPiI2CWrite(i2c_fd, 0xA5);
   printf("wiringPiI2CWrite(0xA5) ==> %d\n", result);
   if (result != 0) {
      printf("  ERRNO = %d (%s)\n", errno, strerror(errno));
   }

// Once we're talking through an I2C splitter, we might not need this, but
// to address a different slave than the one specified in the setup function
// the ioctl(fd, I2C_SLAVE, devId) function might be useful.

#if 0
// Here are the rest of the I2C functions in the Wiring Pi library.
int wiringPiI2CRead(int fd);
int wiringPiI2CWrite(int fd, int data);
int wiringPiI2CWriteReg8(int fd, int reg, int data);
int wiringPiI2CWriteReg16(int fd, int reg, int data);
int wiringPiI2CReadReg8(int fd, int reg);
int wiringPiI2CReadReg16(int fd, int reg);
#endif

   printf("Making RX FIFO...\n");
   result = create_rx_fifo();
   if (result == 0) {
      printf("New FIFO created: %s\n", RX_FIFO);
   } else {
      printf("mkfifo(\"%s\") => %d\n", RX_FIFO, result);
   }

   printf("Making TX FIFO...\n");
   result = create_tx_fifo();
   if (result == 0) {
      printf("New FIFO created: %s\n", TX_FIFO);
   } else {
      printf("mkfifo(\"%s\") => %d\n", TX_FIFO, result);
   }

   printf("Process %d opening FIFO %s\n", getpid(), RX_FIFO);
   rx_fd = open_rx_fifo();
   if (rx_fd != -1) {
      printf("Opened FIFO: %i\n", rx_fd);
   } else {
      printf("Failed to open FIFO %s:\n"
             "open() failed with error [%s]\n",
             RX_FIFO, strerror(errno));
   }

   printf("Process %d opening FIFO %s\n", getpid(), TX_FIFO);
   tx_fd = open_tx_fifo();
   if (tx_fd != -1) {
      printf("Opened FIFO: %i\n", tx_fd);
   } else {
      printf("Failed to open FIFO %s:\n"
             "open() failed with error [%s]\n",
             TX_FIFO, strerror(errno));
   }

   int done = 0;
   do {
      if (rx_fd != -1) {
         const size_t rx_buffer_size = 256;
         unsigned char rx_buffer[rx_buffer_size];
         int rx_length = 
            read(rx_fd, (void *)rx_buffer, sizeof rx_buffer - 1);
         if (rx_length < 0) {
            printf("FIFO Read error\n");
         } else if (rx_length == 0) {
            // No data waiting. Won't get here unless we open RX_FIFO
            // non-blocking.
         } else {
            //Bytes received
            rx_buffer[rx_length] = '\0';
            printf("%s: FIFO %i bytes read: %s\n",
                   argv[0], rx_length, rx_buffer);
            if (rx_buffer[0] == 'q') { // TODO: better exit command
               printf("%s: Setting done = 1\n", argv[0]);
               done = 1;
            }
            write(tx_fd, argv[0], strlen(argv[0]));
            write(tx_fd, "Received: ", sizeof "Received: ");
            write(tx_fd, rx_buffer, rx_length);
         }
      } else {
         done = 1;
      }
   } while (!done);

   close_rx_fifo();
   close_tx_fifo();
}
