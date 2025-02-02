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

#ifndef HEADER__CENTRONIC
#define HEADER__CENTRONIC
#include <stdio.h>
#include "log.h"
#include "nkc.h"

typedef struct {
	FILE *list_file;
	BYTE_68K daten;
	BYTE_68K status;
} cent;

#ifdef __cplusplus
extern "C"
{
#endif

  typedef unsigned short WORD_68K; /* 16 bit unsigned */
  typedef unsigned char BYTE_68K;  /* 8 bit unsigned */

  BYTE_68K cent_p48_in();
  void cent_p48_out(BYTE_68K data);
  BYTE_68K cent_p49_in();
  void cent_p49_out(BYTE_68K data);
  void cent_reset();
  void cent_setFile(const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* HEADER__CENTRONIC */