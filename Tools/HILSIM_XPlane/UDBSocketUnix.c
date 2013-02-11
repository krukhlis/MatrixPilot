//
//  UDBSocketUnix.c
//  MatrixPilot-SIL
//
//  Created by Ben Levitt on 2/1/13.
//  Copyright (c) 2013 MatrixPilot. All rights reserved.
//

#include "UDBSocket.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>


#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#define LOCALHOST_IP "127.0.0.1"


struct UDBSocket_t {
	int					fd;
	struct sockaddr_in	si_other;
	UDBSocketType		type;
	long				UDP_port;
	char*				serial_port;
	long				serial_baud;
} UDBSocket_t;


UDBSocket UDBSocket_init(UDBSocketType type, long UDP_port, char *serial_port, long serial_baud)
{
	UDBSocket newSocket = (UDBSocket)malloc(sizeof(UDBSocket_t));
	if (!newSocket) {
		return NULL;
	}
	
	memset((char *)newSocket, 0, sizeof(UDBSocket_t));
	newSocket->type = type;
	newSocket->UDP_port = UDP_port;
	newSocket->serial_port = (serial_port) ? strdup(serial_port) : NULL;
	newSocket->serial_baud = serial_baud;
	
	switch (newSocket->type) {
		case UDBSocketStandardInOut:
		{
			struct termios ttystate;
			
			//get the terminal state
			tcgetattr(STDIN_FILENO, &ttystate);
			
			//turn off canonical mode
			ttystate.c_lflag &= ~ICANON;
			//minimum of number input read.
			ttystate.c_cc[VMIN] = 1;
			
			//set the terminal attributes.
			tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
			break;
		}
		
		case UDBSocketUDPClient:
		{
			if ((newSocket->fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
				perror("socket() failed");
				free(newSocket);
				return NULL;
			}
			
			int on = 1;
			if (ioctl(newSocket->fd, FIONBIO, (char *)&on) < 0)
			{
				perror("ioctl() failed");
				UDBSocket_close(newSocket);
				return NULL;
			}
			
			memset((char *) &newSocket->si_other, 0, sizeof(newSocket->si_other));
			newSocket->si_other.sin_family = AF_INET;
			newSocket->si_other.sin_port = htons(newSocket->UDP_port);
			if (inet_aton(LOCALHOST_IP, &newSocket->si_other.sin_addr) == 0) {
				fprintf(stderr, "inet_aton() failed\n");
				UDBSocket_close(newSocket);
				return NULL;
			}
			
			UDBSocket_write(newSocket, (unsigned char*)"", 0); // Initiate connection
			
			break;
		}
		
		case UDBSocketUDPServer:
		{
			struct sockaddr_in si_me;
			
			if ((newSocket->fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
				perror("socket");
				free(newSocket);
				return NULL;
			}
			
			int on = 1;
			if (ioctl(newSocket->fd, FIONBIO, (char *)&on) < 0)
			{
				perror("ioctl() failed");
				UDBSocket_close(newSocket);
				return NULL;
			}
			
			newSocket->si_other.sin_family = AF_INET;
			
			memset((char *) &si_me, 0, sizeof(si_me));
			si_me.sin_family = AF_INET;
			si_me.sin_port = htons(newSocket->UDP_port);
			si_me.sin_addr.s_addr = htonl(INADDR_ANY);
			if (bind(newSocket->fd, (const struct sockaddr*)&si_me, sizeof(si_me)) == -1) {
				perror("bind");
				UDBSocket_close(newSocket);
				return NULL;
			}
			break;
		}
		case UDBSocketSerial:
		{
			struct termios  config;
			
			newSocket->fd = open(newSocket->serial_port, O_RDWR | O_NOCTTY | O_NDELAY);
			if (newSocket->fd == -1) {
				perror("serial open() failed");
				return NULL;
			}
			
			if (!isatty(newSocket->fd)) {
				UDBSocket_close(newSocket);
				return NULL;
			}
			
			if (tcgetattr(newSocket->fd, &config) < 0) {
				UDBSocket_close(newSocket);
				return NULL;
			}
			
			//
			// Input flags - Turn off input processing
			// convert break to null byte, no CR to NL translation,
			// no NL to CR translation, don't mark parity errors or breaks
			// no input parity check, don't strip high bit off,
			// no XON/XOFF software flow control
			//
			config.c_iflag &= ~(IGNBRK | BRKINT | ICRNL |
								INLCR | PARMRK | INPCK | ISTRIP | IXON);
			//
			// Output flags - Turn off output processing
			// no CR to NL translation, no NL to CR-NL translation,
			// no NL to CR translation, no column 0 CR suppression,
			// no Ctrl-D suppression, no fill characters, no case mapping,
			// no local output processing
			//
			// config.c_oflag &= ~(OCRNL | ONLCR | ONLRET |
			//                     ONOCR | ONOEOT| OFILL | OLCUC | OPOST);
			config.c_oflag = 0;
			
			//
			// No line processing:
			// echo off, echo newline off, canonical mode off,
			// extended input processing off, signal chars off
			//
			config.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
			
			//
			// Turn off character processing
			// clear current char size mask, no parity checking,
			// no output processing, force 8 bit input
			//
			config.c_cflag &= ~(CSIZE | PARENB | CSTOPB);
			config.c_cflag |= CS8;
			
			//
			// One input byte is enough to return from read()
			// Inter-character timer off
			//
			config.c_cc[VMIN]  = 1;
			config.c_cc[VTIME] = 0;
			
			if (cfsetispeed(&config, newSocket->serial_baud) < 0 || cfsetospeed(&config, newSocket->serial_baud) < 0)
			{
				UDBSocket_close(newSocket);
				return NULL;
			}
			
			//
			// Finally, apply the configuration
			//
			if (tcsetattr(newSocket->fd, TCSAFLUSH, &config) < 0) {
				UDBSocket_close(newSocket);
				return NULL;
			}
			
			break;
		}
		default:
			break;
	}
	
	return newSocket;
}

void UDBSocket_close(UDBSocket socket)
{
	switch (socket->type) {
		case UDBSocketStandardInOut:
		{
			struct termios ttystate;
			
			//get the terminal state
			tcgetattr(STDIN_FILENO, &ttystate);
			
			//turn on canonical mode
			ttystate.c_lflag |= ICANON;
			
			//set the terminal attributes.
			tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);
			break;
		}
		case UDBSocketUDPClient:
		case UDBSocketUDPServer:
		{
			shutdown(socket->fd, SHUT_RDWR);
			close(socket->fd);
			if (socket->serial_port) free(socket->serial_port);
			free(socket);
			break;
		}
		case UDBSocketSerial:
		{
			close(socket->fd);
			if (socket->serial_port) free(socket->serial_port);
			free(socket);
			break;
		}
		default:
			break;
	}
}

int UDBSocket_read(UDBSocket socket, unsigned char *buffer, int bufferLength)
{
	switch (socket->type) {
		case UDBSocketStandardInOut:
		{
			// Check for input on stdin
			struct timeval tv;
			fd_set fds;
			tv.tv_sec = 0;
			tv.tv_usec = 0;
			FD_ZERO(&fds);
			FD_SET(STDIN_FILENO, &fds);
			int pos = 0;
			while (1) {
				select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
				if (FD_ISSET(STDIN_FILENO, &fds)) { // only read stdin if there's data ready
					buffer[pos++] = fgetc(stdin);
				}
				else {
					break;
				}
			}
			return pos;
		}
			
		case UDBSocketUDPClient:
		case UDBSocketUDPServer:
		{
			struct sockaddr_in from;
			socklen_t fromLength = sizeof(from);
			
			int received_bytes = (int)recvfrom(socket->fd, (char*)buffer, bufferLength, 0,
											   (struct sockaddr*)&from, &fromLength);
			
			if ( received_bytes < 0 ) {
				if (errno != EWOULDBLOCK) {
					return -1;
				}
				return 0;
			}
			
			if (socket->type == UDBSocketUDPServer) {
				socket->si_other.sin_port = from.sin_port;
				socket->si_other.sin_addr = from.sin_addr;
			}
			
			return (int)received_bytes;
		}
		case UDBSocketSerial:
		{
			int received_bytes = (int)read(socket->fd, (char*)buffer, bufferLength);
			
			if ( received_bytes < 0 ) {
				if (errno != EWOULDBLOCK) {
					return -1;
				}
			}
			
			if (received_bytes < 0) return 0;
			
			return received_bytes;
		}
		default:
			break;
	}
	return -1;
}


int UDBSocket_write(UDBSocket socket, unsigned char *data, int dataLength)
{
	switch (socket->type) {
		case UDBSocketStandardInOut:
		{
			int i;
			for (i=0; i<dataLength; i++) {
				fputc(data[i], stdout);
			}
			fflush(stdout);
			return i;
		}
		
		case UDBSocketUDPClient:
		case UDBSocketUDPServer:
		{
			if (socket->type == UDBSocketUDPServer && socket->si_other.sin_port == 0) {
				UDBSocket_read(socket, NULL, 0);
				if (socket->si_other.sin_port == 0) {
					return 0;
				}
			}
			
			int bytesWritten = (int)sendto(socket->fd, data, dataLength, 0, (const struct sockaddr*)&socket->si_other, sizeof(socket->si_other));
			if (bytesWritten < 0) {
				perror("sendto()");
				return -1;
			}
			return bytesWritten;
		}
		case UDBSocketSerial:
		{
			int bytesWritten = (int)write(socket->fd, data, dataLength);
			if (bytesWritten < 0) {
				perror("sendto()");
				return -1;
			}
			return bytesWritten;
		}
		default:
			break;
	}
	return -1;
}