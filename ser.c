
/**************************************************************************************
 *   Copyright (C) 2023,2024 by Martin Merck                                          *
 *   martin.merck@gmx.de                                                              *
 *                                                                                    *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy     *
 *   of this software and associated documentation files (the "Software"), to deal    *
 *   in the Software without restriction, including without limitation the rights     *
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell        *
 *   copies of the Software, and to permit persons to whom the Software is            *
 *   furnished to do so, subject to the following conditions:                         *
 *                                                                                    *
 *   The above copyright notice and this permission notice shall be included in all   *
 *   copies or substantial portions of the Software.                                  *
 *                                                                                    *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR       *
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,         * 
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE      *
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER           *
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,    *
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE    *
 *   SOFTWARE.                                                                        *
 *                                                                                    *
 **************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include "ser.h"

BYTE ser_pF0_in()
{
}

void ser_pF0_out(BYTE b)
{

}

// 
void ser_pF3_out(BYTE b)
{
    speed_t brate;
    switch( b & 0x0F) {
        case 0x0:   brate=EXTA;     break;
        case 0x1:   brate=B50;      break;
        case 0x2:   brate=B75;      break;
        case 0x3:   brate=B110;     break;
        case 0x4:   brate=B134;     break;
        case 0x5:   brate=B150;     break;
        case 0x6:   brate=B300;     break;
        case 0x7:   brate=B600;     break;
        case 0x8:   brate=B1200;    break;
        case 0x9:   brate=B1800;    break;
        case 0xA:   brate=B2400;    break;
        case 0xB:   brate=B3600;    break;
        case 0xC:   brate=B4800;    break;
        case 0xD:   brate=B7200;    break;
        case 0xE:   brate=B9600;    break;
        case 0xF:   brate=B19200;   break;
        default:                    break;
    }

    switch( (b & 0x60) >> 5) {
        case 0x0:   brate=CS8;     break;
        case 0x1:   brate=CS7;      break;
        case 0x2:   brate=CS6;      break;
        case 0x3:   brate=CS5;     break;
    }
}

int main(int argc, char *argv[])
{
    int fd;
    char *portname = "/dev/cu.usbserial-0001";
    char buf[256];
    int n;
    int i;
    int count = 0;
    int baudrate = B9600;
    struct termios toptions;

    fd = open(portname, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        perror("serialport_init: Unable to open port ");
        return -1;
    }

    if (tcgetattr(fd, &toptions) < 0) {
        perror("serialport_init: Couldn't get term attributes");
        return -1;
    }
    speed_t brate = baudrate; // let you override switch below if needed
    switch(baudrate) {
        case 4800:   brate=B4800;   break;
        case 9600:   brate=B9600;   break;
        #if defined B14400
        case 14400:  brate=B14400;  break;
        #endif
        case 19200:  brate=B19200;  break;
        #if defined B28800
        case 28800:  brate=B28800;  break;
        #endif
        case 38400:  brate=B38400;  break;
        case 57600:  brate=B57600;  break;
        case 115200: brate=B115200; break;
    }
    cfsetispeed(&toptions, brate);
    cfsetospeed(&toptions, brate);

    // 8N1
    toptions.c_cflag &= ~PARENB;
    toptions.c_cflag &= ~CSTOPB;
    toptions.c_cflag &= ~CSIZE;
    toptions.c_cflag |= CS8;
    // no flow control
    toptions.c_cflag &= ~CRTSCTS;

    toptions.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
    toptions.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl

    toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
//    toptions.c­_oflag &= ~OPOST; // make raw

    toptions.c_cc[VMIN]  = 0;
    toptions.c_cc[VTIME] = 20;

    if( tcsetattr(fd, TCSANOW, &toptions) < 0) {
        perror("init_serialport: Couldn't set term attributes");
        return -1;
    }

    while (1) {
        n = read(fd, buf, 255);
        if (n > 0) {
            buf[n] = 0;
            printf("read %i bytes: %s", n, buf);
        }
        if (count == 0) {
            n = write(fd, "Hello!", 6);
            if (n < 0) {
                perror("Write failed");
            }
            count++;
        }
        usleep(100000);
    }

    return 0;
}