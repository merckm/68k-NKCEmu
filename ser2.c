#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#endif

typedef struct {
#ifdef _WIN32
    HANDLE handle;
#else
    int fd;
#endif
} SerialPort;

SerialPort* serial_open(const char* port_name, int baud_rate) {
    SerialPort* port = (SerialPort*)malloc(sizeof(SerialPort));
    if (!port) return NULL;

#ifdef _WIN32
    port->handle = CreateFile(port_name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (port->handle == INVALID_HANDLE_VALUE) {
        free(port);
        return NULL;
    }

    DCB dcb = {0};
    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(port->handle, &dcb)) {
        CloseHandle(port->handle);
        free(port);
        return NULL;
    }
    dcb.BaudRate = baud_rate;
    dcb.ByteSize = 8;
    dcb.Parity = NOPARITY;
    dcb.StopBits = ONESTOPBIT;
    if (!SetCommState(port->handle, &dcb)) {
        CloseHandle(port->handle);
        free(port);
        return NULL;
    }
#else
    port->fd = open(port_name, O_RDWR | O_NOCTTY | O_SYNC);
    if (port->fd < 0) {
        free(port);
        return NULL;
    }

    struct termios tty;
    if (tcgetattr(port->fd, &tty) != 0) {
        close(port->fd);
        free(port);
        return NULL;
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

    if (tcsetattr(port->fd, TCSANOW, &tty) != 0) {
        close(port->fd);
        free(port);
        return NULL;
    }
#endif

    return port;
}

void serial_close(SerialPort* port) {
    if (port) {
#ifdef _WIN32
        CloseHandle(port->handle);
#else
        close(port->fd);
#endif
        free(port);
    }
}

// Set baud rate
int serial_set_baud_rate(SerialPort* port, int baud_rate) {
#ifdef _WIN32
    DCB dcb = {0};
    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(port->handle, &dcb)) {
        return -1;
    }
    dcb.BaudRate = baud_rate;
    if (!SetCommState(port->handle, &dcb)) {
        return -1;
    }
#else
    struct termios tty;
    if (tcgetattr(port->fd, &tty) != 0) {
        return -1;
    }
    cfsetospeed(&tty, baud_rate);
    cfsetispeed(&tty, baud_rate);
    if (tcsetattr(port->fd, TCSANOW, &tty) != 0) {
        return -1;
    }
#endif
    return 0;
}

// Set character size
int serial_set_char_size(SerialPort* port, int char_size) {
#ifdef _WIN32
    DCB dcb = {0};
    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(port->handle, &dcb)) {
        return -1;
    }
    dcb.ByteSize = char_size;
    if (!SetCommState(port->handle, &dcb)) {
        return -1;
    }
#else
    struct termios tty;
    if (tcgetattr(port->fd, &tty) != 0) {
        return -1;
    }
    tty.c_cflag &= ~CSIZE;
    switch (char_size) {
        case 5: tty.c_cflag |= CS5; break;
        case 6: tty.c_cflag |= CS6; break;
        case 7: tty.c_cflag |= CS7; break;
        case 8: tty.c_cflag |= CS8; break;
        default: return -1;
    }
    if (tcsetattr(port->fd, TCSANOW, &tty) != 0) {
        return -1;
    }
#endif
    return 0;
}

// Set number of stop bits
int serial_set_stop_bits(SerialPort* port, int stop_bits) {
#ifdef _WIN32
    DCB dcb = {0};
    dcb.DCBlength = sizeof(DCB);
    if (!GetCommState(port->handle, &dcb)) {
        return -1;
    }
    switch (stop_bits) {
        case 1: dcb.StopBits = ONESTOPBIT; break;
        case 2: dcb.StopBits = TWOSTOPBITS; break;
        default: return -1;
    }
    if (!SetCommState(port->handle, &dcb)) {
        return -1;
    }
#else
    struct termios tty;
    if (tcgetattr(port->fd, &tty) != 0) {
        return -1;
    }
    switch (stop_bits) {
        case 1: tty.c_cflag &= ~CSTOPB; break;
        case 2: tty.c_cflag |= CSTOPB; break;
        default: return -1;
    }
    if (tcsetattr(port->fd, TCSANOW, &tty) != 0) {
        return -1;
    }
#endif
    return 0;
}

int serial_write(SerialPort* port, const char* data, size_t length) {
#ifdef _WIN32
    DWORD_68K bytes_written;
    if (!WriteFile(port->handle, data, length, &bytes_written, NULL)) {
        return -1;
    }
    return bytes_written;
#else
    return write(port->fd, data, length);
#endif
}

int serial_read(SerialPort* port, char* buffer, size_t length) {
#ifdef _WIN32
    DWORD_68K bytes_read;
    if (!ReadFile(port->handle, buffer, length, &bytes_read, NULL)) {
        return -1;
    }
    return bytes_read;
#else
    return read(port->fd, buffer, length);
#endif
}