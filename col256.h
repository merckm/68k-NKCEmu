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

#ifndef HEADER__COL256
#define HEADER__COL256

#include "nkc.h"

#define COL256_HORIZ_TOT 0
#define COL256_HORIZ_DISP 1
#define COL256_HORIT_SYNC 2
#define COL256_SYNC_WIDTH 3
#define COL256_VERT_TOT 4
#define COL256_VERT_ADJ 5
#define COL256_VERT_DISP 6
#define COL256_VERT_SYNC 7
#define COL256_INTERLACE 8
#define COL256_MAX_RASTER 9
#define COL256_CURSOR_START 10
#define COL256_CURSOR_END 11
#define COL256_START_ADDR_H 12
#define COL256_START_ADDR_L 13
#define COL256_CURSOR_H 14
#define COL256_CURSOR_L 15
#define COL256_LPEN_H 16
#define COL256_LPEN_L 17

typedef struct {
    BYTE col_register[COL256_LPEN_L + 1];
    BYTE col_adr;
    BYTE col_page;
    BYTE col_mem[64 * 1024]; /* 64K Video RAM */
    SDL_Window *col_win;
    SDL_Renderer *col_renderer;
    int col_xmag;              /* Magnification in X */
    int col_ymag;              /* Magnification in Y */
    bool col_active;
    BYTE oldColor;
} col256;

#ifdef __cplusplus
extern "C"
{
#endif

    BYTE col_pCC_in();
    void col_pCC_out(BYTE data);
    BYTE col_pCD_in();
    void col_pCD_out(BYTE data);
    BYTE col_pCE_in();
    void col_pCE_out(BYTE data);
    void col_reset();
    int  col_init();
    void col_setPixel(int address, BYTE data);
    void col_setWord(int address, WORD data);
    void col_setLong(int address, LONG data);
    BYTE col_getPixel(int address);
    WORD col_getWord(int address);
    LONG col_getLong(int address);
    void col_draw();

#ifdef __cplusplus
}
#endif

#endif /* HEADER__COL256 */