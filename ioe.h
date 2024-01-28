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

#ifndef HEADER__IOE
#define HEADER__IOE

#include "nkc.h"
#include "util.h"

#define JOYAXISTHRESHOLD    10000

typedef struct {
    BYTE porta_in;
    BYTE portb_in;
    BYTE porta_out;
    BYTE portb_out;

    char* joy1name;
    char* joy2name;

    SDL_Joystick*   joy1; 
    SDL_Joystick*   joy2;
    SDL_JoystickID  porta_joy_index;
    SDL_JoystickID  portb_joy_index;

	nkc_array * devices; 
} ioe;

#ifdef __cplusplus
extern "C"
{
#endif

    BYTE ioe_p30_in();
    void ioe_p30_out(BYTE data);
    BYTE ioe_p31_in();
    void ioe_p31_out(BYTE data);
    BYTE ioe_get_p30();
    BYTE ioe_get_p31();
    void ioe_reset( const char *joyA, const char *joyB );
    void ioe_event(SDL_Event* event);

#ifdef __cplusplus
}
#endif

#endif /* HEADER__IOE */