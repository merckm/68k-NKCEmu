
/**************************************************************************************
 *   Copyright (C) 2023,2024 by Martin Merck, Andreas Voggeneder                      *
 *   martin.merck@gmx.de                                                              *
 *   andreas_v@gmx.at                                                                 *
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
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdbool.h>
#include "ram_fifo.h"

//#define USE_THREAD

// Status Register bits
#define STATUS_RX_FULL  8       // Receiver full
#define STATUS_TX_EMPTY 0x10    // Transmitter empty

// Command register bits
#define CMD_TX_CTRL 0x0C    // Transmitter steuerung
#define CMD_RX_CTRL 0x10    // Receiver steuerung

static const char RX_UDP_PORT[] ="60001";
static const char TX_UDP_PORT[] ="60002";
static const char IP_ADDR[] ="127.0.0.1";   // Localhost

#ifdef USE_THREAD
    pthread_t rx_thread;
    volatile t_ram_fifo myfifo;
#else
    t_ram_fifo myfifo;
#endif


static ser_registers regs = {0};
int sfd=0;
bool socket_open=false;
struct sockaddr_storage peer_addr_tx={0};

static int resolvehelper(
  const char* hostname,
  int family,
  const char* service,
  struct sockaddr_storage* pAddr) {

    int result;
    struct addrinfo* result_list = NULL;
    struct addrinfo hints = {0};
    hints.ai_family = family;
    hints.ai_socktype = SOCK_DGRAM; // without this flag, getaddrinfo will return 3x the number of addresses (one for each socket type).
    result = getaddrinfo(hostname, service, &hints, &result_list);
    if (result == 0)
    {
        //ASSERT(result_list->ai_addrlen <= sizeof(sockaddr_in));
        memcpy(pAddr, result_list->ai_addr, result_list->ai_addrlen);
        freeaddrinfo(result_list);
    }

    return result;
}
#ifdef USE_THREAD
static void* Receiver(void *arg)
{
    printf("Starting Receiver thread\n");
    while(true) {
        char buf[1024]={0};
        //printf("recvfrom sfd: %d\n",sfd);
        const ssize_t r = recv(sfd, buf, sizeof(buf), MSG_WAITALL);
        if(r>0) {
            //printf("recvfrom done %d\n",(int)r);
            for(int i=0;i<r;i++) {
                write_fifo_value(&myfifo, buf[i]);
            }
        }

    }
    printf("Terminate Receiver thread\n");
    return NULL;
}
#endif

#ifndef USE_THREAD
bool check_data()
{
    fd_set rfds;
    FD_ZERO(&rfds);
    FD_SET(sfd, &rfds);
    struct timeval       tv;
    tv.tv_sec  = 0;
    tv.tv_usec = 0;
    const int retval = select(sfd+1, &rfds, NULL, NULL, &tv);
    if (retval < 0) {
        fprintf(stderr, "select failed\n");
    }
    return (retval!=0);
}

void receive()
{
    if ((sfd>0) && socket_open && check_data()) {
        char buf[1024]={0};
        socklen_t peer_addr_len = sizeof(struct sockaddr_storage);
        const ssize_t nread = recv(sfd, buf, sizeof(buf), MSG_WAITALL);
        //printf("Receive status: %ld 0x%02x\n",nread, buf[0]);
        if (nread != -1) {
            //ret = buf[0];
            // clear Error flags
            regs.status &= ~0x07;
            for(int i=0;i<nread;i++) {
                if (write_fifo_value(&myfifo, buf[i])){
                    regs.status |= 1<<2;        // Überlauf
                    break;
                }
            }
        }
    }
}

#endif
BYTE ser_pF0_in()
{
    BYTE ret = 0;
#ifndef USE_THREAD

    receive();

#endif
    if (socket_open) {
        ret=(BYTE)read_fifo_value(&myfifo);
    }else{
        printf("F0: Warning Socket is not open\n");
    }

    return ret;
}

void ser_pF0_out(BYTE b)
{
    regs.tx_data = b;
    // Trigger UDP Transmit
    const char buf[2]={b,0};
    const ssize_t nwrite=1;
    //printf("Send sfd: %d\n", sfd);
    if ((sfd>0) && socket_open) {
        //printf("Send 0x%02X\n",buf[0]);
        const int result =sendto(
                sfd,
                buf,
                nwrite,
                0,
                (struct sockaddr *) &peer_addr_tx,
                sizeof(struct sockaddr_storage)
            );
            if(result != nwrite) {
                fprintf(stderr, "Error sending data %d / %ld\n", result, nwrite);
            }
    }
}

BYTE ser_pF1_in()
{
    regs.status &= ~STATUS_RX_FULL;  // Clear
    receive();
    if (socket_open) {
        if (!fifo_empty(&myfifo)) {
            regs.status|=STATUS_RX_FULL;
        }

    }else{
        printf("F1: Warning Socket is not open\n");
    }
    return regs.status | STATUS_TX_EMPTY;
}

void close_socket(void)
{
    if (sfd>0) {
        printf("Socket closed\n");
        close(sfd);
        sfd=0;
    }

    if(socket_open) {
        socket_open=false;
#ifdef USE_THREAD
        printf("Try to terminate thread\n");
        const int ret = pthread_cancel(rx_thread);
        if(ret!=0) {
            fprintf(stderr, "Error %d terminating thread\n", ret);
        }
#endif
    }
//#ifdef USE_THREAD
    fifo_destroy(&myfifo);
//#endif
    memset(&peer_addr_tx,0, sizeof(peer_addr_tx));
}

void ser_pF1_out(BYTE b)
{
    // Soft-reset triggered. Reset Configuration to default
    close_socket();
    regs.command &= ~0x1f;  // Reset baudrate
    regs.status = 0x10; // RX,TX-leer
}

void configure_socket(void) {
    struct addrinfo hints = {0};
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
    hints.ai_protocol = 0;          /* Any protocol */
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;

    if(!socket_open) {
//#ifdef USE_THREAD
        init_fifo(&myfifo);
//#endif
        struct addrinfo *result, *rp;

        const int s = getaddrinfo(NULL, RX_UDP_PORT, &hints, &result);
        if (s != 0) {
            close_socket();
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
            return;
        }

        for (rp = result; rp != NULL; rp = rp->ai_next) {
            sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (sfd == -1){
                continue;
            }
            {
#ifndef USE_THREAD
                /*************************************************************/
                /* Set socket to be nonblocking. All of the sockets for      */
                /* the incoming connections will also be nonblocking since   */
                /* they will inherit that state from the listening socket.   */
                /*************************************************************/
                int on=1;
                if (ioctl(sfd, FIONBIO, (char *)&on) < 0)
                {
                    fprintf(stderr,"ioctl() failed");
                    close(sfd);
                    close_socket();
                    freeaddrinfo(result);           // No longer needed
                    return;
                }
#endif
                if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) {
                    fprintf(stderr,"setsockopt(SO_REUSEADDR) failed");
                }
            }
            if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0){
                break;                  /* Success */
            }
            close(sfd);
        }
        if (rp == NULL) {               /* No address succeeded */
            fprintf(stderr, "Could not bind\n");
            close_socket();
            freeaddrinfo(result);           /* No longer needed */
            return;
        }

        if (resolvehelper(IP_ADDR, AF_INET, TX_UDP_PORT, &peer_addr_tx) < 0) {
        //if (resolvehelper("192.168.0.166", AF_INET, "60002", &peer_addr) < 0) {
            fprintf(stderr,"resolvehelper() failed");
        }
        socket_open=true;
#ifdef USE_THREAD
        pthread_create(&rx_thread,NULL,Receiver,NULL);
#endif
        printf("Socket open, sfd: %d\n",sfd);

        freeaddrinfo(result);           /* No longer needed */
    }
}


//
void ser_pF3_out(BYTE b)
{
    regs.control = b;
    printf("ser.control: 0x%02X\n",(unsigned int)b);
    if(((b & 0x70)==0x10) && ((b & 0x0F)!=0)) {
        configure_socket();
    }else{
        close_socket();
    }
}

BYTE ser_pF3_in(void)
{
    return regs.control;
}

void ser_pF2_out(BYTE b)
{
    regs.command = b;
    printf("ser.command: 0x%02X\n",(unsigned int)b);
}

BYTE ser_pF2_in(void)
{
    return regs.command;
}

void ser_close()
{
    close_socket();
}
