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

/**
 * Emulates the HARDCOPY/MOUSE interface.
 */

#include <stdio.h>
#include <stdbool.h>
#include "nkc.h"
#include "mouse.h"
#include "config.h"
#include "log.h"

mouse g_mouse;
extern config g_config;

/*
 * MOUSE functions
 */
void mouse_p88_out(BYTE_68K b)
{
    /* Set MSB of X position for hardcopy */
    g_mouse.hardcopy_x = (g_mouse.hardcopy_x & 0x00FF) | (b << 8);
	return;
}

BYTE_68K mouse_p89_in()
{
     /* return 8-bit port reading */
	return g_mouse.io_port;
}

void mouse_p89_out(BYTE_68K b)
{
    /* Set LSB of X position for hardcopy */
    g_mouse.hardcopy_x = (g_mouse.hardcopy_x & 0xFF00) | b;
	return;
}

BYTE_68K mouse_p8A_in()
{
     /* return ready flag and data-bit */
	return g_mouse.mouse_flags;
}

void mouse_p8A_out(BYTE_68K b)
{
    /* Set MSB of X position for hardcopy */
    g_mouse.hardcopy_y = (g_mouse.hardcopy_y & 0x00FF) | (b << 8);
	return;
}

BYTE_68K mouse_p8B_in()
{
    int32_t x,y;
     /* return buttons of mouse */
    Uint32 buttons = SDL_GetMouseState(&x, &y);
    if( (buttons & SDL_BUTTON_LMASK) != 0 )
        g_mouse.mouse_buttons |= 1;
    else
        g_mouse.mouse_buttons &= 0xFE;
	return g_mouse.mouse_buttons;
}

void mouse_p8B_out(BYTE_68K b)
{
    /* Set LSB of Y position for hardcopy */
    g_mouse.hardcopy_y = (g_mouse.hardcopy_y & 0xFF00) | b;
	return;
}

BYTE_68K mouse_p8C_in()
{
     /* return mouse up counter */
	return g_mouse.mouse_up;
}

BYTE_68K mouse_p8D_in()
{
     /* return mouse down counter */
	return g_mouse.mouse_down;
}

void mouse_p8D_out(BYTE_68K b)
{
    /* Store counters */
    int x,y;
    Uint32 buttons = SDL_GetMouseState(&x, &y);
    if( x > g_mouse.mouse_ref_x ) {
        g_mouse.mouse_right = (x - g_mouse.mouse_ref_x) / g_config.gdp64XMag;
        g_mouse.mouse_left = 0;
    }
    else {
        g_mouse.mouse_left = (g_mouse.mouse_ref_x - x) / g_config.gdp64XMag;
        g_mouse.mouse_right = 0;
    }
    if( y > g_mouse.mouse_ref_y ) {
        g_mouse.mouse_up = (y - g_mouse.mouse_ref_y) / g_config.gdp64YMag;
        g_mouse.mouse_down = 0;
    }
    else {
        g_mouse.mouse_down = (g_mouse.mouse_ref_y - y) / g_config.gdp64YMag;
        g_mouse.mouse_up = 0;
    }
	return;
}

BYTE_68K mouse_p8E_in()
{
     /* return mouse right counter */
	return g_mouse.mouse_right;
}

void mouse_p8E_out(BYTE_68K b)
{
    /* Clear counters */
    int x,y;
    Uint32 buttons = SDL_GetMouseState(&x, &y);
    g_mouse.mouse_ref_x = x;
    g_mouse.mouse_ref_y = y;
	return;
}

BYTE_68K mouse_p8F_in()
{
     /* return mouse left counter */
	return g_mouse.mouse_left;
}

void mouse_event_sdl(SDL_Event* event)
{
    // if (event->type == SDL_MOUSEMOTION)
    // {
	// }
    // if (event->type == SDL_MOUSEBUTTONDOWN ||
    //     event->type == SDL_MOUSEBUTTONUP)
    // {
   	// }
	return;
}

void mouse_reset()
{
    g_mouse.io_port = 0;
    g_mouse.hardcopy_x = 0;
    g_mouse.hardcopy_y = 0;
    g_mouse.mouse_ref_x = 0;
    g_mouse.mouse_ref_y = 0;
	g_mouse.mouse_flags = 0;
	g_mouse.mouse_up = 0;
	g_mouse.mouse_down = 0;
	g_mouse.mouse_right = 0;
	g_mouse.mouse_left = 0;
	g_mouse.mouse_buttons = 0;
	return;
}
