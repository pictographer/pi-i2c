/*
    C socket server example
*/

#include<stdio.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<unistd.h>    //write
#include <errno.h>

// We need a way to tell where we get I2C stuff, alas wiringPiI2C.h doesn't
// define any preprocessor symbols.
#define _WIRINGPII2C_H_
// Might not need to include wiringPi.h because all we need is in
// wiringPiI2C.h.
#include <wiringPi.h>
#include <wiringPiI2C.h>

#include "CCommand.h"

int i2c_fd = -1;

void parse_command() {
    // Split the command into name and numbers.
}

void dispatch_command() {
   // Give every command object a chance to react to the command
}

void report_command_result() {}


int main(int argc, char *argv[])
{
    int socket_desc, client_sock, c, read_size;
    struct sockaddr_in server, client;
    char client_message[2000];

    int result;

    // initialization routine for wiringPI
    wiringPiSetup();
    // indicate i2c interface and target device ID (7 bit address)
    i2c_fd = wiringPiI2CSetupInterface("/dev/i2c-1", 0x70);
    printf("wiringPiI2CSetup() ==> %d\n", i2c_fd);

    // set up mux for channel 0
    unsigned char data = wiringPiI2CReadReg8(i2c_fd, 0x00);
    printf("wiringPiI2CReadReg8(0x00) ==> %d\n", data);
    result = wiringPiI2CWriteReg8(i2c_fd, 0x00, 0x08);
    printf("wiringPiI2CWriteReg8(0x00,0x08) ==> %d\n", result);
    if (result != 0) {
       printf("  ERRNO = %d (%s)\n", errno, strerror(errno));
    }

// Once we're talking through an I2C splitter, we might not need this, but
// to address a different slave than the one specified in the setup function
// the ioctl(fd, I2C_SLAVE, devId) function might be useful.

// Here are the rest of the I2C functions in the Wiring Pi library.
// int wiringPiI2CRead(int fd);
// int wiringPiI2CWrite(int fd, int data);
// int wiringPiI2CWriteReg8(int fd, int reg, int data);
// int wiringPiI2CWriteReg16(int fd, int reg, int data);
// int wiringPiI2CReadReg8(int fd, int reg);
// int wiringPiI2CReadReg16(int fd, int reg);

#if 0
    //Create socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");

    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 5432 );

    //Bind
    if (bind(socket_desc,(struct sockaddr *)&server, sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");

    //Listen
    listen(socket_desc, 3);

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);

    //accept connection from an incoming client
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
    puts("Connection accepted");

    //Receive a message from client
    while( (read_size = recv(client_sock, client_message, 2000, 0)) > 0 )
    {
        printf("%s\n",client_message);
        //Send the message back to client
        sprintf(client_message,"BT2I:234.56;BT1I:543.56;BRDV:4.00;btti:678.43;vout:4.00;iout:9.87;time:125454;baro_p:6.54;baro_t:3.21;");
        write(client_sock, client_message, strlen(client_message));
    }

    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
#else
    CCommand cmd;
    bool gotCommand = cmd.GetCommandString();
    printf(gotCommand ? "Got command.\n" : "Didn't get command.\n");


#endif
    return 0;
}

