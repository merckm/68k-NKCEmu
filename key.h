
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

#ifndef HEADER__KEY
#define HEADER__KEY

#include "nkc.h"

typedef struct
{
    BYTE_68K keyReg68; /* Status */
    BYTE_68K keyReg69; /* DIL settings */

    char *clipboardText;
    size_t clipboardLength;
    size_t clipboardOffset;
} key;

#ifdef __cplusplus
extern "C"
{
#endif

    typedef unsigned short WORD_68K; /* 16 bit unsigned */
    typedef unsigned char BYTE_68K;  /* 8 bit unsigned */

    // BYTE_68K keyReg68 = 0x80; /* initialize with a value which has bit 7 set */
    // BYTE_68K keyReg69 = 0xE7; /* DIL settings, here standard is used */

    BYTE_68K key_p68_in();
    void key_p68_out(BYTE_68K data);
    BYTE_68K key_p69_in();
    void key_p69_out(BYTE_68K data);
    void key_reset();
    void key_event(SDL_Event *event);

#ifdef __cplusplus
}
#endif

#endif /* HEADER__KEY */