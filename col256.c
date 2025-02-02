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
 * Emulates a COL256 graphic card in 256x256 mode.
 * Other modes are currently not supported
 */
#include <stdio.h>
#include <unistd.h>
#include "col256.h"
#include "config.h"
#include "log.h"

col256 g_col;
extern config g_config;

// MC6845 Address Register
// Can not be read
BYTE_68K col_pCC_in()
{
    return 0;
}

void col_pCC_out(BYTE_68K data)
{
    g_col.col_adr = data & 0x1F;
}

// MC6845 Data Register adressed by Adress egister
// Can obnly sometimes be read, currently not supported
BYTE_68K col_pCD_in()
{
    return 0;
}

void col_pCD_out(BYTE_68K data)
{
    switch (g_col.col_adr)
    {
    case COL256_HORIZ_TOT:
        g_col.col_register[COL256_HORIZ_TOT] = data;
        break;
    case COL256_HORIZ_DISP:
        g_col.col_register[COL256_HORIZ_DISP] = data;
        break;
    case COL256_HORIT_SYNC:
        g_col.col_register[COL256_HORIT_SYNC] = data;
        break;
    case COL256_SYNC_WIDTH:
        g_col.col_register[COL256_SYNC_WIDTH] = data;
        break;
    case COL256_VERT_TOT:
        g_col.col_register[COL256_VERT_TOT] = data;
        break;
    case COL256_VERT_ADJ:
        g_col.col_register[COL256_VERT_ADJ] = data;
        break;
    case COL256_VERT_DISP:
        g_col.col_register[COL256_VERT_DISP] = data;
        break;
    case COL256_VERT_SYNC:
        g_col.col_register[COL256_VERT_SYNC] = data;
        break;
    case COL256_INTERLACE:
        g_col.col_register[COL256_INTERLACE] = data;
        break;
    case COL256_MAX_RASTER:
        g_col.col_register[COL256_MAX_RASTER] = data;
        break;
    case COL256_CURSOR_START:
        g_col.col_register[COL256_CURSOR_START] = data;
        break;
    case COL256_CURSOR_END:
        g_col.col_register[COL256_CURSOR_END] = data;
        break;
    case COL256_START_ADDR_H:
        g_col.col_register[COL256_START_ADDR_H] = data;
        break;
    case COL256_START_ADDR_L:
        g_col.col_register[COL256_START_ADDR_L] = data;
        break;
    case COL256_CURSOR_H:
        g_col.col_register[COL256_CURSOR_H] = data;
        break;
    case COL256_CURSOR_L:
        g_col.col_register[COL256_CURSOR_L] = data;
        break;
    case COL256_LPEN_H:
        g_col.col_register[COL256_LPEN_H] = data;
        break;
    case COL256_LPEN_L:
        g_col.col_register[COL256_LPEN_L] = data;
        break;
    default:
        break;
    }

    return;
}

// Col256 page select
BYTE_68K col_pCE_in()
{
    return g_col.col_page;
}

void col_pCE_out(BYTE_68K data)
{
    if ((data & 0x80) != 0)
        g_col.col_active = true;
    else
        g_col.col_active = false;

    g_col.col_page = data & 0x03;
}

void col_reset()
{
    g_col.col_page = 0;
    g_col.col_active = false;
    g_col.col_adr = 0;
}

int col_init(void)
{
    g_col.col_xmag = g_config.col256XMag; /* Magnification in X */
    g_col.col_ymag = g_config.col256YMag; /* Magnification in Y */
    g_col.col_active = FALSE;

//    int rendererFlags = SDL_RENDERER_ACCELERATED;
    int rendererFlags = SDL_RENDERER_SOFTWARE;
    int windowFlags = 0;

    g_col.col_win = SDL_CreateWindow("COL256", 200 + 512 * g_config.gdp64XMag, 100, 256 * g_col.col_xmag, 256 * g_col.col_ymag, windowFlags);
    if (!g_col.col_win)
    {
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "Failed to open COL256 window: %s", SDL_GetError());
        exit(1);
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    g_col.col_renderer = SDL_CreateRenderer(g_col.col_win, -1, rendererFlags);
    if (!g_col.col_renderer)
    {
        SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "Failed to create renderer: %s", SDL_GetError());
        exit(1);
    }
    g_col.oldColor = 0;

    return SDL_GetWindowID(g_col.col_win);
}

void col_setPixel(int address, BYTE_68K data)
{
    int x, y;
    Uint8 R, G, B;

    if (!g_col.col_active)
        return;

    int addr = g_col.col_page * 0x4000 + (address - g_config.col256RAMAddr);
    g_col.col_mem[addr] = data;
    x = addr % 256;
    y = addr / 256;

    int intens = ((data & 0xC0) >> 6);
    Uint8 R8 = (data & 0x03);
    Uint8 G8 = ((data & 0x0C) >> 2);
    Uint8 B8 = ((data & 0x30) >> 4);
    R = R8 * 64 + (intens * 21);
    G = G8 * 64 + (intens * 21);
    B = B8 * 64 + (intens * 21);
    SDL_SetRenderDrawColor(g_col.col_renderer, R, G, B, SDL_ALPHA_OPAQUE);
    for (int i = 0; i < g_col.col_xmag; i++)
    {
        for (int j = 0; j < g_col.col_ymag; j++)
        {
            SDL_RenderDrawPoint(g_col.col_renderer, x * g_col.col_xmag + i, y * g_col.col_ymag + j);
        }
    }
}

void col_setWord(int address, WORD_68K data)
{
    BYTE_68K hi = (data & 0xFF00) >> 8;
    BYTE_68K lo = (data & 0x00FF);
    col_setPixel(address, hi);
    col_setPixel(address + 1, lo);
}

void col_setLong(int address, LONG_68K data)
{
    BYTE_68K byte1 = (data & 0xFF000000) >> 24;
    BYTE_68K byte2 = (data & 0x00FF0000) >> 16;
    BYTE_68K byte3 = (data & 0x0000FF00) >> 8;
    BYTE_68K byte4 = (data & 0x000000FF);
    col_setPixel(address, byte1);
    col_setPixel(address + 1, byte2);
    col_setPixel(address + 2, byte3);
    col_setPixel(address + 3, byte4);
}

BYTE_68K col_getPixel(int address)
{
    if (!g_col.col_active)
        return 0;

    int addr = g_col.col_page * 0x4000 + (address - 0xEC000);
    return g_col.col_mem[addr];
}

WORD_68K col_getWord(int address)
{
    if (!g_col.col_active)
        return 0;

    int addr = g_col.col_page * 0x4000 + (address - 0xEC000);
    WORD_68K res = g_col.col_mem[addr] << 8;
    res += g_col.col_mem[addr + 1];
    return res;
}

LONG_68K col_getLong(int address)
{
    if (!g_col.col_active)
        return 0;

    int addr = g_col.col_page * 0x4000 + (address - 0xEC000);
    LONG_68K res = g_col.col_mem[addr] << 24;
    res += g_col.col_mem[addr + 1] << 16;
    res += g_col.col_mem[addr + 2] << 8;
    res += g_col.col_mem[addr + 3];
    return res;
}

void col_draw()
{
    SDL_RenderPresent(g_col.col_renderer);
}
