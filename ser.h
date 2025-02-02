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

#ifndef HEADER__SER
#define HEADER__SER
#include "log.h"
#include "nkc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

typedef struct {
    SerialPort* port;
    BYTE_68K receive_data;
    BYTE_68K transmit_data;
    BYTE_68K status;
    BYTE_68K command;
    BYTE_68K control;
    int brate;
    int char_size;
    int stop_bits;
    bool interrupt_enable;
} ser;

#ifdef __cplusplus
extern "C"
{
#endif

    BYTE_68K ser_pF0_in();
    void ser_pF0_out(BYTE_68K data);
    BYTE_68K ser_pF1_in();
    void ser_pF1_out(BYTE_68K data);
    BYTE_68K ser_pF2_in();
    void ser_pF2_out(BYTE_68K data);
    BYTE_68K ser_pF3_in();
    void ser_pF3_out(BYTE_68K data);
    void ser_reset();
    void ser_setPort(const char *portname);

#ifdef __cplusplus
}
#endif

#endif /* HEADER__SER */