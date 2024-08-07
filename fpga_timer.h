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

#ifndef HEADER__FPGA_TIMER
#define HEADER__FPGA_TIMER
#include <termios.h>
#include "nkc.h"

typedef struct {
    BYTE ctrl;
    uint16_t timer;
    uint16_t reload;
} fpga_timer_registers;

#ifdef __cplusplus
extern "C"
{
#endif

    BYTE bus_fpga_timer_in(const BYTE ofs);
    void bus_fpga_timer_out(const BYTE ofs, const BYTE data);
    bool handle_timer(const int diff);


#ifdef __cplusplus
}
#endif

#endif /* HEADER__FPGA_TIMER */