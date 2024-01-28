/**************************************************************************************
 *   Original GDP64 code by Torsten Evers                                             *
 *   Copyright (C) 2007 by Torsten Evers                                              *
 *   tevers@onlinehome.de                                                             *
 *                                                                                    *
 *   Adaption for 68008 CPU by Martin Merck                                           *
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

#ifndef HEADER__GDP64
#define HEADER__GDP64

#include "nkc.h"

typedef struct {
    BYTE status;                /* status of the gdp */
    BYTE ctrl1;                 /* CTRL1 register */
    BYTE ctrl2;                 /* CTRL2 register */
    int penX;                   /* Pen X and Y position */
    int penY;
    BYTE csize;                 /* CSIZE register */
    BYTE deltax;                /* DELTAX register */
    BYTE deltay;                /* DELTAY register */
    BYTE seite;                 /* PAGE register */
} gdp64_registers;

typedef struct {
    int x;
    int y;
    BYTE csize;
    int counter;
    bool isVisible;
    bool isOff;
} gdp64_gui_cursor;


typedef struct {
    int xmag;                   /* Magnification in X */
    int ymag;                   /* Magnification in Y */
    gdp64_registers regs;       /* GDP-Registers      */
    gdp64_registers save_regs;  /* GDP-Registers      */
    int actualWritePage;        /* on which page do we write at the moment? */
    int actualReadPage;         /* which page is shown at the moment? */
    int contentChanged;         /* something new written? */
    bool isGuiScreen;           /* remember if GUI screen */
    SDL_Window   *window;
    SDL_Renderer *renderer;
    SDL_Surface  *pages[5];
} gdp64;

#ifdef __cplusplus
extern "C"
{
#endif

    BYTE gdp64_p60_in();
    void gdp64_p60_out(BYTE data);
    BYTE gdp64_p70_in();
    void gdp64_p70_out(BYTE data);
    BYTE gdp64_p71_in();
    void gdp64_p71_out(BYTE data);
    BYTE gdp64_p72_in();
    void gdp64_p72_out(BYTE data);
    void gdp64_p72_out_word(int data);
    BYTE gdp64_p73_in();
    void gdp64_p73_out(BYTE data);
    BYTE gdp64_p75_in();
    void gdp64_p75_out(BYTE data);
    BYTE gdp64_p77_in();
    void gdp64_p77_out(BYTE data);
    BYTE gdp64_p78_in();
    int gdp64_p78_in_word();
    void gdp64_p78_out(BYTE data);
    void gdp64_p78_out_word(int data);
    BYTE gdp64_p79_in();
    void gdp64_p79_out(BYTE data);
    BYTE gdp64_p7A_in();
    int gdp64_p7A_in_word();
    void gdp64_p7A_out(BYTE data);
    void gdp64_p7A_out_word(int data);
    BYTE gdp64_p7B_in();
    void gdp64_p7B_out(BYTE data);
    void gdp64_gui_draw_string(int x, int y, int size, const char * str);
    void gdp64_gui_input_string(int x, int y, int size, int len, char * str);
    int  gdp64_init();
    void gdp64_reset();
    void gdp64_clear_screen();
    void gdp64_save_regs();
    void gdp64_restore_regs();
    void gdp64_set_vsync(BYTE vs);
    void gdp64_event(SDL_Event* event);

#ifdef __cplusplus
}
#endif

#endif /* HEADER__GDP64 */