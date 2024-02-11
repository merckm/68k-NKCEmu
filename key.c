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
 * Emulates the KEY interface.
 * Most keys on modern keyboards should map directly to the corresponding key of
 * the NDR-KleinComputer.
 * This interface also supports the Clipboard to paste text into an editor.
 * As the Ctrl-Keys are needed for special functions in the NDR-Klein Computer
 * the Insert Key is mapped to paste clipboard content. 
 */

#include <stdio.h>
#include <stdbool.h>
#include "nkc.h"
#include "key.h"
#include "config.h"
#include "util.h"
#include "log.h"

key g_key;
extern config g_config;

void clipboard_reset()
{
    // free clipoard text if all has benn pasted
    if ( g_key.clipboardText != NULL )
    {
        SDL_free(g_key.clipboardText);
        g_key.clipboardText = NULL;
        g_key.clipboardLength = 0;
        g_key.clipboardOffset = 0;
    }

}

void set_key(BYTE key)
{
	g_key.keyReg68 = key;
}

/*
 * KEY functions
 */
BYTE key_p68_in()
{
    // free clipoard text if all has benn pasted
    if ( (g_key.clipboardText != NULL)
         &&
         (g_key.clipboardOffset >= g_key.clipboardLength) )
    {
		clipboard_reset();
    }

    /* check if clipboard text can still be pasted */
    if (g_key.clipboardOffset < g_key.clipboardLength)
    {
        BYTE ret = g_key.clipboardText[g_key.clipboardOffset];
        // We will increase the offset when we receive a strobe reset
        if (ret == 0x0A)     // Ignore LF as it is Ctrl-J
        {
            g_key.clipboardOffset++;    // Here we will need to increase the offset
            ret = 0x80;
        }
        return ret;
    }

    // If no clipboard test to process return actual key from keyboard
	return g_key.keyReg68;
}

void key_p68_out(BYTE b)
{
    log_warn("Key output not supported");
	return; /* no output on key ports */
}

BYTE key_p69_in()
{
    /* Increment clipboard offset on strob bit reset */
    if (g_key.clipboardOffset < g_key.clipboardLength)
    {
        g_key.clipboardOffset++;
    }

	g_key.keyReg68 = 0x80; /* reset key strobe bit to no key pressed */
	return g_key.keyReg69; /* return DIP switch settings */
}

void key_p69_out(BYTE b)
{
	return; /* no output on key DIP ports */
}

void key_event(SDL_Event* event)
{
    if (event->type == SDL_KEYUP)
    {
        BYTE released = nkc_get_ascii(event->key);
		if( released == g_key.keyReg68 )
			g_key.keyReg68 = 0x80; /* reset status to no key pressed */
	}
    if (event->type == SDL_KEYDOWN)
    {
		if (event->key.keysym.sym == SDLK_INSERT)
		{
			if (SDL_HasClipboardText() == SDL_TRUE)
			{
				if (g_key.clipboardText != NULL)
				{
					SDL_free(g_key.clipboardText);
				}
				g_key.clipboardText = SDL_GetClipboardText();
				g_key.clipboardLength = strlen(g_key.clipboardText);
				g_key.clipboardOffset = 0;
			}
		}

        g_key.keyReg68 = nkc_get_ascii(event->key);
	}
	return;
}

void key_reset()
{
	g_key.keyReg68 = 0x80; /* reset status to no key pressed */
    g_key.keyReg69 = g_config.keyDILSwitches; /* DIL settings from config */
	clipboard_reset();
	return;
}
