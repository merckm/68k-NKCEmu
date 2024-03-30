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

#ifndef HEADER__MOUSE
#define HEADER__MOUSE
#include <termios.h>
#include "nkc.h"

typedef struct {
    BYTE key_stat;
    BYTE up_reg;
    BYTE down_reg;
    BYTE right_reg;
    BYTE left_reg;
} mouse_registers;

#define MOUSE_BASE_ADDRESS 0x88

#ifdef __cplusplus
extern "C"
{
#endif

    void mouse_event(SDL_Event *event);
    BYTE bus_mouse_in(const BYTE ofs);
    void bus_mouse_out(const BYTE ofs, const BYTE data);


#ifdef __cplusplus
}
#endif

#endif /* HEADER__MOUSE */