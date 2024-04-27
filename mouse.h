
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

#ifndef HEADER__MOUSE
#define HEADER__MOUSE

#include "nkc.h"

#define MOUSE_BUTTON_LEFT 0x01
#define MOUSE_BUTTON_MIDDLE 0x02
#define MOUSE_BUTTON_RIGHT 0x04
#define MOUSE_BUTTON_X1 0x08
#define MOUSE_BUTTON_X2 0x10

typedef struct
{
    BYTE io_port;
    uint16_t hardcopy_x;
    uint16_t hardcopy_y;
    int32_t mouse_ref_x;
    int32_t mouse_ref_y;
    uint16_t mouse_buttons;
    BYTE mouse_flags;
    BYTE mouse_up;
    BYTE mouse_down;
    BYTE mouse_left;
    BYTE mouse_right;
} mouse;

#ifdef __cplusplus
extern "C"
{
#endif

    typedef unsigned short WORD; /* 16 bit unsigned */
    typedef unsigned char BYTE;  /* 8 bit unsigned */

    // BYTE keyReg68 = 0x80; /* initialize with a value which has bit 7 set */
    // BYTE keyReg69 = 0xE7; /* DIL settings, here standard is used */

    // BYTE mouse_p88_in();                // not used
    void mouse_p88_out(BYTE data);
    BYTE mouse_p89_in();
    void mouse_p89_out(BYTE data);
    BYTE mouse_p8A_in();
    void mouse_p8A_out(BYTE data);
    BYTE mouse_p8B_in();
    void mouse_p8B_out(BYTE data);
    BYTE mouse_p8C_in();
    // void mouse_p8C_out(BYTE data);      // not used
    BYTE mouse_p8D_in();
    void mouse_p8D_out(BYTE data);
    BYTE mouse_p8E_in();
    void mouse_p8E_out(BYTE data);
    BYTE mouse_p8F_in();
    // void mouse_p8F_out(BYTE data);      // not used
    void mouse_reset();
    void mouse_event(SDL_Event *event);

#ifdef __cplusplus
}
#endif

#endif /* HEADER__KEY */