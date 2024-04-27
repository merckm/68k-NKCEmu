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
 * Emulates the IOE interface.
 * IOE Inputs are derived from the Keys 1-8 if used with the Alt-Key kodifier.
 * Alternatively configured Joysticks can be used. (Inputs 1-5)
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>
#include "ioe.h"
#include "log.h"

ioe g_ioe;

// Inernal helpers
BYTE getTasten(SDL_KeyboardEvent keyEvent)
{
    SDL_Keycode keycode = keyEvent.keysym.sym;
    Uint16 mod = keyEvent.keysym.mod;

    bool isAlt = (mod & KMOD_LALT) != 0;

    switch (keycode)
    {
    case SDLK_UP:
        return 0x04; // Up -> Bit 1
    case SDLK_DOWN:
        return 0x08; // Down -> Bit 2
    case SDLK_LEFT:
        return 0x02; // Left -> Bit 3
    case SDLK_RIGHT:
        return 0x01; // Right -> Bit 4
	case SDLK_KP_ENTER:
        return 0x10; // Fire -> Bit 5
    case SDLK_1:
        return (isAlt ? 0x01 : 0x00);
    case SDLK_2:
        return (isAlt ? 0x02 : 0x00);
    case SDLK_3:
        return (isAlt ? 0x04 : 0x00);
    case SDLK_4:
        return (isAlt ? 0x08 : 0x00);
    case SDLK_5:
        return (isAlt ? 0x10 : 0x00);
    case SDLK_6:
        return (isAlt ? 0x20 : 0x00);
    case SDLK_7:
        return (isAlt ? 0x40 : 0x00);
    case SDLK_8:
        return (isAlt ? 0x80 : 0x00);
	default:
		return 0x00;
	}
	return 0x00;
}

void list_joysticks( const char *joyA, const char *joyB ) {
    int res;

    int num_joysticks = SDL_NumJoysticks();
    if (num_joysticks == 0) {
        log_warn("No joysticks were found\n");
    } else {
        log_info("Found %d joystick(s)", num_joysticks);
        for(int joy_idx = 0; joy_idx < num_joysticks; ++joy_idx) {
            SDL_Joystick* joy = SDL_JoystickOpen(joy_idx);
            const char* name = SDL_JoystickName(joy);
            if (!joy) {
                log_warn("Unable to open joystick %d", joy_idx);
            } else {
                bool joy_set = false;
                nkc_arr_appendString(g_ioe.devices,name);
                if( joyA != NULL ) 
                {
                    res = strncmp( name, joyA, strlen(joyA)); 
                    if ( res == 0)
                    {
                        g_ioe.joy1 = joy;
                        g_ioe.porta_joy_index = SDL_JoystickInstanceID(joy);
                        joy_set = true;
                    }
                }
                if( joyB != NULL ) 
                {
                    res = strncmp( name, joyB, strlen(joyB));
                    if ( res == 0)
                    {
                        if( !( SDL_JoystickInstanceID(joy) == g_ioe.porta_joy_index ) )
                        {
                            g_ioe.joy2 = joy;
                            g_ioe.portb_joy_index = SDL_JoystickInstanceID(joy);
                            joy_set = true;
                        }
                    }
                }
                if( ! joy_set )
                    SDL_JoystickClose(joy);
            }
        }
    }
    if( g_ioe.joy1 != NULL )
        log_info("Port A Joystick: %s", SDL_JoystickName(g_ioe.joy1));
    if( g_ioe.joy2 != NULL )
        log_info("Port B Joystick: %s", SDL_JoystickName(g_ioe.joy2));
}

/*
 * IOE functions
 */
BYTE ioe_p30_in()
{
	return ~g_ioe.porta_in;
}

void ioe_p30_out(BYTE b)
{
	g_ioe.porta_out = b;
    log_debug("Port A output: %d", b);
	return; /* no output */
}

BYTE ioe_p31_in()
{
	return g_ioe.portb_in;
}

void ioe_p31_out(BYTE b)
{
	g_ioe.portb_out = b;
    log_debug("Port B output: %d", b);
	return;
}

BYTE ioe_get_p30()
{
    return g_ioe.porta_out;
}

BYTE ioe_get_p31()
{
    return g_ioe.portb_out;
}

void ioe_event(SDL_Event* event)
{
    if (event->type == SDL_KEYDOWN)
    {
        BYTE mask = getTasten(event->key);
        g_ioe.porta_in = g_ioe.porta_in | mask;
        g_ioe.portb_in = g_ioe.portb_in | mask;
    }
    if (event->type == SDL_KEYUP)
    {
        BYTE mask = ~getTasten(event->key);
        g_ioe.porta_in = g_ioe.porta_in & mask;
        g_ioe.portb_in = g_ioe.portb_in & mask;
    }
    if (event->type == SDL_JOYAXISMOTION) {
        BYTE* ioe_port;
//        log_debug("Joystick axis %d - %d", event->jaxis.axis, event->jaxis.value);
        if ( event->jaxis.which == g_ioe.porta_joy_index )
            ioe_port = &g_ioe.porta_in;
        if ( event->jaxis.which == g_ioe.portb_joy_index )
            ioe_port = &g_ioe.portb_in;        
        if ( event->jaxis.which == g_ioe.porta_joy_index 
            ||
            event->jaxis.which == g_ioe.portb_joy_index )
        {
            if( event->jaxis.axis == 0)       // Left-right axis
            {
                if( event->jaxis.value > JOYAXISTHRESHOLD )       // right
                    *ioe_port = (*ioe_port) | 0x01; 
                else if( event->jaxis.value < -JOYAXISTHRESHOLD ) // left
                    *ioe_port = (*ioe_port) | 0x02; 
                else                                             // neutral
                    *ioe_port = (*ioe_port) & 0xF3; 
            }   
            if( event->jaxis.axis == 1)       // up-down axis
            {
                if( event->jaxis.value > JOYAXISTHRESHOLD )       // down
                    *ioe_port = (*ioe_port) | 0x08; 
                else if( event->jaxis.value < -JOYAXISTHRESHOLD ) // up
                    *ioe_port = (*ioe_port) | 0x04; 
                else                                              // neutral
                    *ioe_port = (*ioe_port) & 0xFC; 
            }   
        }
    }
    if ( (event->type == SDL_JOYBUTTONDOWN)
         ||
         (event->type == SDL_JOYBUTTONUP) ) {
        BYTE* ioe_port;
        if ( event->jaxis.which == g_ioe.porta_joy_index )
            ioe_port = &g_ioe.porta_in;
        if ( event->jaxis.which == g_ioe.portb_joy_index )
            ioe_port = &g_ioe.portb_in;        
        if ( event->jaxis.which == g_ioe.porta_joy_index 
            ||
            event->jaxis.which == g_ioe.portb_joy_index )
        {
            int button = event->jbutton.button;
            if( button < 4) 
            {
                if( event->type == SDL_JOYBUTTONDOWN)       // Button pressed
                {
                    BYTE mask = 1 << (4 + button );
                    *ioe_port = (*ioe_port) | mask; 
                    log_debug("Joystick button: %i pressed", event->jbutton.button);
                }
                if( event->type == SDL_JOYBUTTONUP)       // Button released
                {
                    BYTE mask = 1 << (4 + button );
                    mask = 0xFF ^ mask;
                    *ioe_port = (*ioe_port) & mask; 
                    log_debug("Joystick button: %i released", event->jbutton.button);
                }
            }
        }   
    }
}

void ioe_reset( const char *joyA, const char *joyB )
{
	g_ioe.porta_in = 0;
	g_ioe.portb_in = 0;
	g_ioe.porta_out = 0;
	g_ioe.portb_out = 0;
    if( g_ioe.joy1 != NULL ) {
        SDL_JoystickClose(g_ioe.joy1);
    }
    if( g_ioe.joy2 != NULL ) {
        SDL_JoystickClose(g_ioe.joy2);
    }
    g_ioe.porta_joy_index = -1;
    g_ioe.portb_joy_index = -1;

    if(g_ioe.devices != NULL)
        nkc_arr_free(g_ioe.devices);
    g_ioe.devices =  nkc_arr_new();


    list_joysticks(joyA, joyB);
	return;
}

