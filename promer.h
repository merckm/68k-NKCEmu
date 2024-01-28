
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

#ifndef HEADER__PROMER
#define HEADER__PROMER
#include <stdio.h>
#include <time.h>
#include "nkc.h"

typedef struct {
	FILE *prom_file;
    BYTE daten;
    WORD adr;
    int size;
    bool led;
    bool read;
    struct timeval stoptime;
    struct timeval progtime;
} promer;

#ifdef __cplusplus
extern "C"
{
#endif

    typedef unsigned short WORD; /* 16 bit unsigned */
    typedef unsigned char BYTE;  /* 8 bit unsigned */

    BYTE promer_p80_in();
    void promer_p80_out(BYTE data);
    BYTE promer_p81_in();
    void promer_p81_out(BYTE data);
    BYTE promer_p82_in();
    void promer_p82_out(BYTE data);
    void promer_reset();
    void promer_setFile(const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* HEADER__PROMER */