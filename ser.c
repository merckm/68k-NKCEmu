
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
#include "ser.h"
#include "config.h"

ser g_ser;

extern config g_config;

void ser_open(const char *port_name, int baud_rate)
{
    log_debug("SER: Open serial port %s", port_name);
    g_ser.port = (SerialPort *)malloc(sizeof(SerialPort));
    if (!g_ser.port)
    {
        log_error("SER: Failed to allocate memory for serial port\n");
        return;
    }

#ifdef _WIN32
    g_ser.port->handle = CreateFile(port_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (g_ser.port->handle == INVALID_HANDLE_VALUE)
    {
        free(g_ser.port);
        g_ser.port = NULL;
        log_error("SER: Failed to open port %s\n", port_name);
        return;
    }

    DCB dcb = {0};
    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(g_ser.port->handle, &dcb))
    {
        CloseHandle(g_ser.port->handle);
        free(g_ser.port);
        g_ser.port = NULL;
    }
    dcb.BaudRate = baud_rate;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    if (!SetCommState(g_ser.port->handle, &dcb))
    {
        CloseHandle(g_ser.port->handle);
        free(g_ser.port);
        g_ser.port = NULL;
    }
#else
    g_ser.port->fd = open(port_name, O_RDWR | O_NOCTTY | O_SYNC);
    if (g_ser.port->fd < 0)
    {
        free(g_ser.port);
        g_ser.port = NULL;
    }

    struct termios tty;
    if (tcgetattr(g_ser.port->fd, &tty) != 0)
    {
        close(g_ser.port->fd);
        free(g_ser.port);
        g_ser.port = NULL;
    }

    cfsetospeed(&tty, baud_rate);
    cfsetispeed(&tty, baud_rate);
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_iflag &= ~IGNBRK;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 5;
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~(PARENB | PARODD);
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(g_ser.port->fd, TCSANOW, &tty) != 0)
    {
        close(g_ser.port->fd);
        free(g_ser.port);
        g_ser.port = NULL;
    }
#endif
}

void ser_close()
{
    log_debug("SER: Close serial port");
    if (g_ser.port)
    {
#ifdef _WIN32
        CloseHandle(g_ser.port->handle);
#else
        close(g_ser.port->fd);
#endif
        free(g_ser.port);
    }
}

// Set baud rate
int ser_set_baud_rate(int baud_rate)
{
    log_debug("SER: Set baud rate to %d", baud_rate);
#ifdef _WIN32
    DCB dcb = {0};
    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(g_ser.port->handle, &dcb))
    {
        return -1;
    }
    dcb.BaudRate = baud_rate;
    if (!SetCommState(g_ser.port->handle, &dcb))
    {
        return -1;
    }
#else
    struct termios tty;
    if (tcgetattr(g_ser.port->fd, &tty) != 0)
    {
        return -1;
    }
    cfsetospeed(&tty, baud_rate);
    cfsetispeed(&tty, baud_rate);
    if (tcsetattr(g_ser.port->fd, TCSANOW, &tty) != 0)
    {
        return -1;
    }
#endif
    return 0;
}

// Set character size
int ser_set_char_size(int char_size)
{
    log_debug("SER: Set char size to %d", char_size);
#ifdef _WIN32
    DCB dcb = {0};
    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(g_ser.port->handle, &dcb))
    {
        return -1;
    }
    dcb.ByteSize = char_size;
    if (!SetCommState(g_ser.port->handle, &dcb))
    {
        return -1;
    }
#else
    struct termios tty;
    if (tcgetattr(g_ser.port->fd, &tty) != 0)
    {
        return -1;
    }
    tty.c_cflag &= ~CSIZE;
    switch (char_size)
    {
    case 5:
        tty.c_cflag |= CS5;
        break;
    case 6:
        tty.c_cflag |= CS6;
        break;
    case 7:
        tty.c_cflag |= CS7;
        break;
    case 8:
        tty.c_cflag |= CS8;
        break;
    default:
        return -1;
    }
    if (tcsetattr(g_ser.port->fd, TCSANOW, &tty) != 0)
    {
        return -1;
    }
#endif
    return 0;
}

// Set number of stop bits
int ser_set_stop_bits(int stop_bits)
{
    log_debug("SER: Set stop bits to %d", stop_bits);
#ifdef _WIN32
    DCB dcb = {0};
    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(g_ser.port->handle, &dcb))
    {
        return -1;
    }
    switch (stop_bits)
    {
    case 1:
        dcb.StopBits = ONESTOPBIT;
        break;
    case 2:
        dcb.StopBits = TWOSTOPBITS;
        break;
    default:
        return -1;
    }
    if (!SetCommState(g_ser.port->handle, &dcb))
    {
        return -1;
    }
#else
    struct termios tty;
    if (tcgetattr(g_ser.port->fd, &tty) != 0)
    {
        return -1;
    }
    switch (stop_bits)
    {
    case 1:
        tty.c_cflag &= ~CSTOPB;
        break;
    case 2:
        tty.c_cflag |= CSTOPB;
        break;
    default:
        return -1;
    }
    if (tcsetattr(g_ser.port->fd, TCSANOW, &tty) != 0)
    {
        return -1;
    }
#endif
    return 0;
}

int serial_write(const char *data)
{
#ifdef _WIN32
    DWORD bytes_written;
    if (!WriteFile(g_ser.port->handle, &g_ser.transmit_data, 1L, &bytes_written, NULL))
    {
        log_error("SER: Write failed\n");
    }
    return bytes_written;
#else
    return write(g_ser.port->fd, data, 1L);
#endif
}

int serial_read(char *buffer)
{
#ifdef _WIN32
    DWORD bytes_read;
    COMSTAT comStat;
    DWORD errors;
    ClearCommError(g_ser.port->handle, &errors, &comStat);

    if (comStat.cbInQue > 0) {
        if (!ReadFile(g_ser.port->handle, buffer, 1L, &bytes_read, NULL))
        {
            return -1;
        }
        log_debug("SER: Read %d bytes", bytes_read);
        return bytes_read;
    }
    return -1;
#else
    return read(g_ser.port->fd, buffer, length);
#endif
}

/// @brief Read the data register with received data
/// @return Content of the data register
BYTE_68K ser_pF0_in()
{
    log_debug("SER: Data register read %02x", g_ser.receive_data);
    g_ser.status &= 0xF7; // Set receive buffer empty (Bit 3 is zero)
    return g_ser.receive_data;
}

/// @brief Set the data register with data to be transmitted
/// @param b Data to be transmitted
void ser_pF0_out(BYTE_68K b)
{
    log_debug("SER: Data register set to %02x", b);
    g_ser.transmit_data = b;
    g_ser.status &= 0xEF; // Set transmit buffer full (Bit 4 is zero)
    if (g_ser.port != NULL)
    {
        serial_write(&g_ser.transmit_data);
    }
    g_ser.status |= 0x10; // Set transmit buffer empty (Bit 4 is one)
}

/// @brief Read the status register
/// @return The status register
BYTE_68K ser_pF1_in()
{
    // log_debug("SER: Status register read %02x", g_ser.status);
    // first check if there is data to read
    if (g_ser.port != NULL)
    {
        if ((g_ser.status & 0x08) == 0x00)        // if receive buffer is empty
        {
            char buffer;
            if (serial_read(&buffer) > 0)
            {
                log_debug("SER: Received data %02x", buffer);
                g_ser.receive_data = buffer;
                g_ser.status |= 0x08;           // set receive buffer full
            }
        }
    }
    return g_ser.status;
}

/// @brief Write the status register triggers a reset
/// @param b any value
void ser_pF1_out(BYTE_68K b)
{
    log_debug("SER: Status register set to %02x", b);
    g_ser.status = 0x10;
    g_ser.interrupt_enable = false;
    ser_reset();
}

/// @brief Read the command register
/// @return Content of the command register
BYTE_68K ser_pF2_in()
{
    log_debug("SER: Command register read %02x\n", g_ser.command);
    return g_ser.command;
}

/// @brief Set the command register
/// @param b New content of the command register
void ser_pF2_out(BYTE_68K b)
{
    log_debug("SER: Command register set to %02x", b);
    g_ser.command = b;
    // if( b & 0x01 )
    // {
    //     g_ser.status |= 0x01;
    //     g_ser.interrupt_enable = true;
    // }
    // else
    // {
    //     g_ser.status &= ~0x01;
    //     g_ser.interrupt_enable = false;
    // }
}

/// @brief Set the control register
/// @param b New content of the control register
void ser_pF3_out(BYTE_68K b)
{
    log_debug("SER: Control register set to %02x", b);
    g_ser.control = b;
    switch (b & 0x0F)
    {
    case 0x0:
        g_ser.brate = 0;
        break;
    case 0x1:
        g_ser.brate = 50;
        break;
    case 0x2:
        g_ser.brate = 75;
        break;
    case 0x3:
        g_ser.brate = 110;
        break;
    case 0x4:
        g_ser.brate = 134;
        break;
    case 0x5:
        g_ser.brate = 150;
        break;
    case 0x6:
        g_ser.brate = 300;
        break;
    case 0x7:
        g_ser.brate = 600;
        break;
    case 0x8:
        g_ser.brate = 1200;
        break;
    case 0x9:
        g_ser.brate = 1800;
        break;
    case 0xA:
        g_ser.brate = 2400;
        break;
    case 0xB:
        g_ser.brate = 3600;
        break;
    case 0xC:
        g_ser.brate = 4800;
        break;
    case 0xD:
        g_ser.brate = 7200;
        break;
    case 0xE:
        g_ser.brate = 9600;
        break;
    case 0xF:
        g_ser.brate = 19200;
        break;
    default:
        break;
    }

    switch ((b & 0x60) >> 5)
    {
    case 0x0:
        g_ser.char_size = 8;
        break;
    case 0x1:
        g_ser.char_size = 7;
        break;
    case 0x2:
        g_ser.char_size = 6;
        break;
    case 0x3:
        g_ser.char_size = 5;
        break;
    }

    switch ((b & 0x80))
    {
    case 0x0:
        g_ser.stop_bits = 1;
        break;
    case 0x80:
        g_ser.stop_bits = 2;
        break;
    }

    ser_set_baud_rate(g_ser.brate);
    ser_set_char_size(g_ser.char_size);
    ser_set_stop_bits(g_ser.stop_bits);
}

/// @brief Read the control register
/// @return Content of the control register
BYTE_68K ser_pF3_in()
{
    log_debug("SER: Control register read %02x\n", g_ser.control);
    return g_ser.control;
}

void ser_reset()
{
    g_ser.receive_data = 0;
    g_ser.transmit_data = 0;
    g_ser.status = 0x10; // Set transmit buffer empty
    g_ser.command = 0;
    g_ser.control = 0;
    g_ser.brate = 0;
    g_ser.char_size = 0;
    g_ser.stop_bits = 0;
    g_ser.interrupt_enable = false;
    ser_setPort("COM3");
}

void ser_setPort(const char *portname)
{
    log_debug("SER: Set port to %s\n", portname);
    ser_close(g_ser.port);
    ser_open(portname, 9600);
}
