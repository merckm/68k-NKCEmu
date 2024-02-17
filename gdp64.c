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

/**
 * Emulates a GDP64 graphics card with a Thomson EF9366 graphics processor
 * it uses the SDL2 library for graphical output
 * since handling of key events is also done by SDL lib, KEY functions
 * reside in this file too
 */
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "ef9366charset.h"
#include "gdp64.h"
#include "log.h"
#include "util.h"
#include "config.h"
#include "m68k.h"

extern config g_config;

extern void resetVsyncTimer();

// struct timeval g_oldtime;
// struct timeval g_akttime;

gdp64 g_gdp;
gdp64_gui_cursor g_gui_cursor;

WORD lineStyle[4] = {0xFFFF,  /* Line styles of EF9366 in hexadecimal representation, first a continous line */
                     0xCCCC,  /* Dotted (2 dots on, 2 dots off) */
                     0xF0F0,  /* Dashed (4 dots on, 4 dots off) */
                     0xFFCC}; /* Dotted-dashed (10 dots on, 2 dots off, 2 dots on, 2 dots off) */

/* global variables SDL */
const SDL_Color bg = {0x00, 0x00, 0x00, 0xFF};
const Uint32 bg32 = 0x000000FF;
const SDL_Color fg = {0x10, 0xA4, 0x13, 0xFF};
const Uint32 fg32 = 0x10A413FF;

/*
  Begin of all SDL related functions
*/
void DrawPixel(SDL_Surface *sc, int x, int y, Uint8 R, Uint8 G, Uint8 B)
{
    if (x < 0 || x > 511)
        return; /* Pen is outside of the screen */
    if (y < 0 || y > 255)
        return; /* same in y direction */
    // XOR mode active when (1.) Draw-pen is selected and (2.) XOR_EN bit is set
    const bool xor_en = (((g_gdp.regs.seite & 0x01)!=0) && ((g_gdp.regs.ctrl1 & 0x02)!=0));

    Uint32 color = SDL_MapRGB(sc->format, R, G, B);

    switch (sc->format->BytesPerPixel)
    {
    case 1:
    { /* vermutlich 8 Bit */
        Uint8 *bufp;

        for (int i = 0; i < g_gdp.xmag; i++)
        {
            for (int j = 0; j < g_gdp.ymag; j++)
            {
                bufp = (Uint8 *)sc->pixels + (y * g_gdp.ymag + j) * sc->pitch + (x * g_gdp.xmag + i);
                if (xor_en) {
                    *bufp ^= color;
                }else{
                    *bufp = color;
                }
            }
        }
    }
    break;

    case 2:
    { /* vermutlich 15 Bit oder 16 Bit */
        Uint16 *bufp;

        for (int i = 0; i < g_gdp.xmag; i++)
        {
            for (int j = 0; j < g_gdp.ymag; j++)
            {
                bufp = (Uint16 *)sc->pixels + (y * g_gdp.ymag + j) * sc->pitch / 2 + (x * g_gdp.xmag + i);
                if (xor_en) {
                    *bufp ^= color;
                }else{
                    *bufp = color;
                }
            }
        }
    }
    break;

    case 3:
    { /* langsamer 24-Bit-Modus, selten verwendet */
        Uint8 *bufp;

        for (int i = 0; i < g_gdp.xmag; i++)
        {
            for (int j = 0; j < g_gdp.ymag; j++)
            {
                bufp = (Uint8 *)sc->pixels + (y * g_gdp.ymag + j) * sc->pitch + (x * g_gdp.xmag + i) * 3;
                bufp = (Uint8 *)sc->pixels + (y * g_gdp.ymag + j) * sc->pitch + (x * g_gdp.xmag + i);
                if (SDL_BYTEORDER == SDL_LIL_ENDIAN)
                {
                    if (xor_en) {
                        bufp[0] ^= color;
                        bufp[1] ^= (color >> 8);
                        bufp[2] ^= (color >> 16);
                    }else{
                        bufp[0] = color;
                        bufp[1] = color >> 8;
                        bufp[2] = color >> 16;
                    }
                }
                else
                {
                    if (xor_en) {
                        bufp[2] ^= color;
                        bufp[1] ^= (color >> 8);
                        bufp[0] ^= (color >> 16);
                    }else{
                        bufp[2] = color;
                        bufp[1] = color >> 8;
                        bufp[0] = color >> 16;
                    }
                }
            }
        }
    }
    break;

    case 4:
    { /* vermutlich 32 Bit */
        Uint32 *bufp;

        for (int i = 0; i < g_gdp.xmag; i++)
        {
            for (int j = 0; j < g_gdp.ymag; j++)
            {
                bufp = (Uint32 *)sc->pixels + (y * g_gdp.ymag + j) * sc->pitch / 4 + (x * g_gdp.xmag + i);
                if (xor_en) {
                    *bufp ^= color;
                }else{
                    *bufp = color;
                }
            }
        }
    }
    break;
    }
    g_gdp.contentChanged = 1;
}

void DrawChar(unsigned char c)
{
//    log_debug("GDP64: Draw char %c %d  at %d-%d", c, c, g_gdp.regs.penX, g_gdp.regs.penY);

    /* draws a char at the current position of the pen */
    int realX = g_gdp.regs.penX;        /* x coordinate is running exactly as it's on the PC */
    int realY = 255 - g_gdp.regs.penY;  /* PC has y-coordinate 0 in the upper left corner, NKC has it in the lower left */
    unsigned char c_off = c - ' '; /* calulate down to array base */
    int x, y;                      /* loop variables */
    int xSize = 0;
    int ySize = 0;

    if (c_off > 96)
        return;          /* wasn't an ASCII character or block */
    if (g_gdp.regs.ctrl1 & 1) /* is Pen down? else just calculate new coordinates */
    {
        if (SDL_MUSTLOCK(g_gdp.pages[g_gdp.actualWritePage]))
        {
            if (SDL_LockSurface(g_gdp.pages[g_gdp.actualWritePage]) < 0)
            {
                return;
            }
        }
        /* is pen in writing or deleting mode? */
        SDL_Color pen;
        if (g_gdp.regs.ctrl1 & 2)
            pen = fg;
        else
            pen = bg;

        /* calculate sizing in X and Y directions */
        xSize = (g_gdp.regs.csize & 0xF0) >> 4;
        if (xSize == 0)
            xSize = 16;
        ySize = (g_gdp.regs.csize & 0x0F);
        if (ySize == 0)
            ySize = 16;
        for (x = 0; x < 5; x++)
        {
            for (y = 0; y < 8; y++)
            {
                if ((charset[c_off][x] & 128 >> y) != 0)
                {
                    // draw pixel block, depending on magnification
                    for (int x1 = 0; x1 < xSize; x1++)
                    {
                        for (int y1 = 0; y1 < ySize; y1++)
                        {
                            if (g_gdp.regs.ctrl2 & 8)
                                DrawPixel(g_gdp.pages[g_gdp.actualWritePage], realX - y * ySize - y1, realY - x * xSize - x1, pen.r, pen.g, pen.b);
                            else
                                DrawPixel(g_gdp.pages[g_gdp.actualWritePage], realX + x * xSize + x1, realY - y * ySize - y1, pen.r, pen.g, pen.b);
                        }
                    }
                }
                if (g_gdp.regs.ctrl2 & 4)
                {
                    if (g_gdp.regs.ctrl2 & 8)
                        realY--;
                    else
                        realX++;
                }
            }
            if (g_gdp.regs.ctrl2 & 4) // tilted
            {
                if (g_gdp.regs.ctrl2 & 8)
                    realY += 8;
                else
                    realX -= 8;
            }
        }

        if (SDL_MUSTLOCK(g_gdp.pages[g_gdp.actualWritePage]))
        {
            SDL_UnlockSurface(g_gdp.pages[g_gdp.actualWritePage]);
        }
    }
    // Now correct penX and penY
    if (g_gdp.regs.ctrl2 & 8)
        g_gdp.regs.penY += 6 * xSize;
    else
        g_gdp.regs.penX += 6 * xSize; // char width + 1
}

void DrawBlock()
{
    /* draws a char at the current position of the pen */
    int realX = g_gdp.regs.penX; /* x coordinate is running exactly as it's on the PC */
    int realY = 255 - g_gdp.regs.penY; /* PC has y-coordinate 0 in the upper left corner, NKC has it in the lower left */
    int x, y;               /* loop variables */
    int xSize = 0;
    int ySize = 0;

    if (g_gdp.regs.ctrl1 & 1) /* is Pen down? else just calculate new coordinates */
    {
        if (SDL_MUSTLOCK(g_gdp.pages[g_gdp.actualWritePage]))
        {
            if (SDL_LockSurface(g_gdp.pages[g_gdp.actualWritePage]) < 0)
            {
                return;
            }
        }
        /* is pen in writing or deleting mode? */
        SDL_Color pen;
        if (g_gdp.regs.ctrl1 & 2)
            pen = fg;
        else
            pen = bg;
        /* calculate sizing in X and Y directions */
        xSize = (g_gdp.regs.csize & 0xF0) >> 4;
        ySize = (g_gdp.regs.csize & 0x0F);
        if (xSize == 0)
            xSize = 16;
        if (ySize == 0)
            ySize = 16;

        for (x = 0; x < 4; x++)
        {
            for (y = 0; y < 4; y++)
            {
                int x1 = 0;
                int y1 = 0;
                // draw pixel block, depending on magnification
                for (x1 = 0; x1 < xSize; x1++)
                {
                    for (y1 = 0; y1 < ySize; y1++)
                    {
                        if (g_gdp.regs.ctrl2 & 8)
                            DrawPixel(g_gdp.pages[g_gdp.actualWritePage], realX - y * ySize - y1, realY - x * xSize - x1, pen.r, pen.g, pen.b);
                        else
                            DrawPixel(g_gdp.pages[g_gdp.actualWritePage], realX + x * xSize + x1, realY - y * ySize - y1, pen.r, pen.g, pen.b);
                    }
                }
                if (g_gdp.regs.ctrl2 & 4)
                {
                    if (g_gdp.regs.ctrl2 & 8)
                        realY--;
                    else
                        realX++;
                }
            }
            if (g_gdp.regs.ctrl2 & 4) // tilted
            {
                if (g_gdp.regs.ctrl2 & 8)
                    realY += 8;
                else
                    realX -= 8;
            }
        }

        if (SDL_MUSTLOCK(g_gdp.pages[g_gdp.actualWritePage]))
        {
            SDL_UnlockSurface(g_gdp.pages[g_gdp.actualWritePage]);
        }
    }
    // Now correct penX and penY
    g_gdp.regs.penX += 4 * xSize; // char width + 1
}

/*
 Draws a horizontal Line from x1,y to x2,y
 */
void DrawHLine(int x1, int y, int x2)
{
    /* which line style do we use? */
    WORD style = lineStyle[(g_gdp.regs.ctrl2 & 3)];
    g_gdp.regs.penX = x2; /* adjust X coordinate even if pen is up */
    /* should wie exchange x values? */
    if (x2 < x1)
    {
        int h = x1;
        x1 = x2;
        x2 = h; /* triangular exchange */
    }
    WORD bit = 0x8000;   /* bit mask for line style */
    if (g_gdp.regs.ctrl1 & 1) /* is pen up? */
    {
        if (SDL_MUSTLOCK(g_gdp.pages[g_gdp.actualWritePage]))
        {
            if (SDL_LockSurface(g_gdp.pages[g_gdp.actualWritePage]) < 0)
            {
                return;
            }
        }
        /* is pen in writing or deleting mode? */
        SDL_Color pen;
        SDL_Color inv;
        if (g_gdp.regs.ctrl1 & 2)
        {
            pen = fg;
            inv = bg;
        }
        else
        {
            pen = bg;
            inv = fg;
        }
        for (; x1 <= x2; x1++)
        {
            if (style & bit)
                DrawPixel(g_gdp.pages[g_gdp.actualWritePage], x1, y, pen.r, pen.g, pen.b);
            else
                DrawPixel(g_gdp.pages[g_gdp.actualWritePage], x1, y, inv.r, inv.g, inv.b);
            bit = (bit >> 1);
            if (bit == 0)
                bit = 0x8000;
        }
        if (SDL_MUSTLOCK(g_gdp.pages[g_gdp.actualWritePage]))
        {
            SDL_UnlockSurface(g_gdp.pages[g_gdp.actualWritePage]);
        }
    }
}

/*
 Draws a vertical Line from x,y1 to x,y2
 */
void DrawVLine(int x, int y1, int y2)
{
    /* which line style do we use? */
    WORD style = lineStyle[(g_gdp.regs.ctrl2 & 3)];
    g_gdp.regs.penY = 255 - y2; /* adjust Y coordinate even if pen is up */
    /* should wie exchange x values? */
    if (y2 < y1)
    {
        int h = y1;
        y1 = y2;
        y2 = h; /* triangular exchange */
    }
    WORD bit = 0x8000;   /* bit mask for line style */
    if (g_gdp.regs.ctrl1 & 1) /* is pen up? */
    {
        if (SDL_MUSTLOCK(g_gdp.pages[g_gdp.actualWritePage]))
        {
            if (SDL_LockSurface(g_gdp.pages[g_gdp.actualWritePage]) < 0)
            {
                return;
            }
        }
        /* is pen in writing or deleting mode? */
        SDL_Color pen;
        SDL_Color inv;
        if (g_gdp.regs.ctrl1 & 2)
        {
            pen = fg;
            inv = bg;
        }
        else
        {
            pen = bg;
            inv = fg;
        }
        for (; y1 <= y2; y1++)
        {
            if (style & bit)
                DrawPixel(g_gdp.pages[g_gdp.actualWritePage], x, y1, pen.r, pen.g, pen.b);
            else
                DrawPixel(g_gdp.pages[g_gdp.actualWritePage], x, y1, inv.r, inv.g, inv.b);
            bit = (bit >> 1);
            if (bit == 0)
                bit = 0x8000;
        }
        if (SDL_MUSTLOCK(g_gdp.pages[g_gdp.actualWritePage]))
        {
            SDL_UnlockSurface(g_gdp.pages[g_gdp.actualWritePage]);
        }
    }
}

/*
 Draws a Line from x1,y1 to x2,y2 using the bresenham algorithm
 adopted from an implementation of
 */
void DrawLine(int x1, int y1, int x2, int y2)
{
    /* which line style do we use? */
    WORD style = lineStyle[(g_gdp.regs.ctrl2 & 3)];
    /* adjust coordinates, even if pen is up */
    // g_gdp.regs.penY=255-y2;
    // g_gdp.regs.penX=x2;

    /* should wie exchange x values? */
    if (x2 < x1)
    {
        int h = y1;
        y1 = y2;
        y2 = h; /* triangular exchange */
        h = x1;
        x1 = x2;
        x2 = h;
    }
    WORD bit = 0x8000;   /* bit mask for line style */
    if (g_gdp.regs.ctrl1 & 1) /* is pen up? */
    {
        if (SDL_MUSTLOCK(g_gdp.pages[g_gdp.actualWritePage]))
        {
            if (SDL_LockSurface(g_gdp.pages[g_gdp.actualWritePage]) < 0)
            {
                return;
            }
        }
        /* is pen in writing or deleting mode? */
        SDL_Color pen;
        SDL_Color inv;
        if (g_gdp.regs.ctrl1 & 2)
        {
            pen = fg;
            inv = bg;
        }
        else
        {
            pen = bg;
            inv = fg;
        }
        /* now start bresenham */
        int dx = abs(x2 - x1);
        int dy = abs(y2 - y1);
        int inc_dec = ((y2 >= y1) ? 1 : -1);

        if (dx > dy)
        {
            int two_dy = (2 * dy);
            int two_dy_dx = (2 * (dy - dx));
            int p = ((2 * dy) - dx);

            int x = x1;
            int y = y1;

            if (style & bit)
                DrawPixel(g_gdp.pages[g_gdp.actualWritePage], x, y, pen.r, pen.g, pen.b);
            else
                DrawPixel(g_gdp.pages[g_gdp.actualWritePage], x, y, inv.r, inv.g, inv.b);
            bit = (bit >> 1);
            if (bit == 0)
                bit = 0x8000;

            while (x < x2)
            {
                x++;

                if (p < 0)
                    p += two_dy;

                else
                {
                    y += inc_dec;
                    p += two_dy_dx;
                }

                if (style & bit)
                    DrawPixel(g_gdp.pages[g_gdp.actualWritePage], x, y, pen.r, pen.g, pen.b);
                else
                    DrawPixel(g_gdp.pages[g_gdp.actualWritePage], x, y, inv.r, inv.g, inv.b);
                bit = (bit >> 1);
                if (bit == 0)
                    bit = 0x8000;
            }
        }
        else
        {
            int two_dx = (2 * dx);
            int two_dx_dy = (2 * (dx - dy));
            int p = ((2 * dx) - dy);

            int x = x1;
            int y = y1;

            if (style & bit)
                DrawPixel(g_gdp.pages[g_gdp.actualWritePage], x, y, pen.r, pen.g, pen.b);
            else
                DrawPixel(g_gdp.pages[g_gdp.actualWritePage], x, y, inv.r, inv.g, inv.b);
            bit = (bit >> 1);
            if (bit == 0)
                bit = 0x8000;

            while (y != y2)
            {
                y += inc_dec;

                if (p < 0)
                    p += two_dx;

                else
                {
                    x++;
                    p += two_dx_dy;
                }

                if (style & bit)
                    DrawPixel(g_gdp.pages[g_gdp.actualWritePage], x, y, pen.r, pen.g, pen.b);
                else
                    DrawPixel(g_gdp.pages[g_gdp.actualWritePage], x, y, inv.r, inv.g, inv.b);
                bit = (bit >> 1);
                if (bit == 0)
                    bit = 0x8000;
            }
        }
        if (SDL_MUSTLOCK(g_gdp.pages[g_gdp.actualWritePage]))
        {
            SDL_UnlockSurface(g_gdp.pages[g_gdp.actualWritePage]);
        }
    }
}

void clearScreen()
{
    /* fills the SDL surface with the background color */
    SDL_FillRect(g_gdp.pages[g_gdp.actualWritePage], NULL, SDL_MapRGB(g_gdp.pages[g_gdp.actualWritePage]->format, bg.r, bg.g, bg.b));
}

void fillScreen()
{
    /* fills the SDL surface with the background color */
    SDL_FillRect(g_gdp.pages[g_gdp.actualWritePage], NULL, SDL_MapRGB(g_gdp.pages[g_gdp.actualWritePage]->format, fg.r, fg.g, fg.b));
}

/*
 Begin of I/O functions for port 0x60 and 0x70-0x7F
*/

BYTE gdp64_p60_in()
{
    return g_gdp.regs.seite;
}

void gdp64_p60_out(BYTE b)
{
//    if(g_gdp.isGuiScreen)
//        return;
    g_gdp.actualReadPage = (b & 0x30) >> 4;
    g_gdp.actualWritePage = (b & 0xC0) >> 6;
    if (g_gdp.regs.seite != b)
    {
        g_gdp.contentChanged = 1;
    }
    g_gdp.regs.seite = b;
}

BYTE gdp64_p61_in()
{
    return 0;   // Should be DMA Register
}

void gdp64_p61_out(BYTE b)
{
//    if(g_gdp.isGuiScreen)
//        return;
    if (g_gdp.regs.scroll != b)
    {
        g_gdp.contentChanged = 1;

    }
    g_gdp.regs.scroll = b;
}

BYTE gdp64_p70_in()
{
    /* read status register of EF9366
     * meanings of the bits:
     * bit 0: status of lightpen sequence 1 means that a sequence just ended
     * bit 1: vertical sync signal, 1 means it's vertical sync time
     * bit 2: ready status, 0 means no command my be given, 1 means ready
     * bit 3: position of the writing pen, 1 means out of the screen borders
     * bit 4: lightpen sequence IRQ
     * bit 5: vertical sync IRQ
     * bit 6: ready sigbal IRQ
     * bit 7: ORed bits 4-7
     */
    // Calculate state of 20ms vsync signal (VB)
    // VB is active for 1.472 milliseconds on each frame
    // We get the nanoseconds of the clock and calculate how many nsec have
    // passed since the start of the frame. During the first 1472000 nano seconds
    // we assert the VB signal.

    // struct timespec ts;
    // timespec_get(&ts, TIME_UTC);
    // long nsec = ts.tv_nsec - (ts.tv_nsec / 20000000) * 20000000;
    // if (nsec < 1472000)
    //     return (g_gdp.regs.status | 0x02);
    // else
    return g_gdp.regs.status;
}

void gdp64_p70_out(BYTE b)
{
    // if(b < 0x20 )
    //     log_info("GDP64: Write to port 0x70: %x", b);
    Uint32 pen;
    g_gdp.regs.status = (g_gdp.regs.status & 0xFB);
    /* accept commands for the EF9366 and call the SDL implementations for them */
    if (b >= 0x20 && b <= 0x7F) /* was an ASCII character */
    {
        DrawChar(b);
        g_gdp.regs.status = (g_gdp.regs.status | 4);
        return;
    }
    /* short vector command? */
    if (b >= 128)
    {
        signed char dirMul[][2] = {{1, 0}, {1, 1}, {0, 1}, {-1, 1}, {0, -1}, {1, -1}, {-1, 0}, {-1, -1}};
        BYTE dx = (b & 0x60) >> 5;
        BYTE dy = ((b & 0x18) >> 3);
        BYTE dir = (b & 7);
        int i;
        if (g_gdp.regs.ctrl1 & 2)
            pen = fg32;
        else
            pen = bg32;

        DrawLine(g_gdp.regs.penX, 255 - g_gdp.regs.penY, g_gdp.regs.penX + dx * dirMul[dir][0], 255 - (g_gdp.regs.penY + dy * dirMul[dir][1]));
        g_gdp.contentChanged = 1;
        g_gdp.regs.penX = g_gdp.regs.penX + dx * dirMul[dir][0];
        g_gdp.regs.penY = g_gdp.regs.penY + dy * dirMul[dir][1];
        g_gdp.regs.status = g_gdp.regs.status | 4;
        return;
    }
    /* must be a GDP command */
    switch (b)
    {
    case 0: /* pen selection */
        g_gdp.regs.ctrl1 = (g_gdp.regs.ctrl1 | 2);
        break;

    case 1: /* eraser selection */
        g_gdp.regs.ctrl1 = (g_gdp.regs.ctrl1 & 0xFD);
        break;

    case 2: /* pen down */
        g_gdp.regs.ctrl1 = (g_gdp.regs.ctrl1 | 1);
        break;

    case 3: /* pen up */
        g_gdp.regs.ctrl1 = (g_gdp.regs.ctrl1 & 0xFE);
        break;

    case 4: /* clear screen */
        clearScreen();
        break;

    case 5: /* set X and Y registers to 0 */
        g_gdp.regs.penX = 0;
        g_gdp.regs.penY = 0;
        break;

    case 6: /* clear screen and reset coordinates */
        clearScreen();
        g_gdp.regs.penX = 0;
        g_gdp.regs.penY = 0;
        break;

    case 7: /* clear screen, set CSIZE to 1, other registers to 0 */
        clearScreen();
        g_gdp.regs.penX = 0;
        g_gdp.regs.penY = 0;
        g_gdp.regs.csize = 0x11;
        g_gdp.regs.ctrl1 = 0;
        g_gdp.regs.status = 4;
        break;

    case 10: /* block drawing 5x8 */
        DrawChar(128);
        break;

    case 11: /* block drawing 4x4 */
        DrawBlock();
        break;

    case 12: /* screen scanning */
        if ( (g_gdp.regs.ctrl1 & 2) != 0 )
            fillScreen();
        else
            clearScreen();
        break;

    case 13: /* set X to 0 */
        g_gdp.regs.penX = 0;
        break;

    case 14: /* set Y to 0 */
    log_info("==== STATRT ERASE  ===== GDP64: Set Y to 0");
        g_gdp.regs.penY = 0;
        break;

    case 16: /* draw horizontal line in positive x direction */
        DrawHLine(g_gdp.regs.penX, 255 - g_gdp.regs.penY, g_gdp.regs.penX + g_gdp.regs.deltax);
        g_gdp.contentChanged = 1;
        break;

    case 17: /* draw line in positive x and y direction */
        if (g_gdp.regs.ctrl1 & 2)
            pen = fg32;
        else
            pen = bg32;

        DrawLine(g_gdp.regs.penX, 255 - g_gdp.regs.penY, g_gdp.regs.penX + g_gdp.regs.deltax, 255 - (g_gdp.regs.penY + g_gdp.regs.deltay));
        g_gdp.contentChanged = 1;
        g_gdp.regs.penX = g_gdp.regs.penX + g_gdp.regs.deltax;
        g_gdp.regs.penY = g_gdp.regs.penY + g_gdp.regs.deltay;
        break;

    case 18: /* draw vertical line in positive y direction */
        DrawVLine(g_gdp.regs.penX, 255 - g_gdp.regs.penY, 255 - (g_gdp.regs.penY + g_gdp.regs.deltay));
        g_gdp.contentChanged = 1;
        break;

    case 19: /* draw line in negative x and positive y direction */
        if (g_gdp.regs.ctrl1 & 2)
            pen = fg32;
        else
            pen = bg32;

        DrawLine(g_gdp.regs.penX, 255 - g_gdp.regs.penY, g_gdp.regs.penX - g_gdp.regs.deltax, 255 - (g_gdp.regs.penY + g_gdp.regs.deltay));
        g_gdp.contentChanged = 1;
        g_gdp.regs.penX = g_gdp.regs.penX - g_gdp.regs.deltax;
        g_gdp.regs.penY = g_gdp.regs.penY + g_gdp.regs.deltay;
        break;

    case 20: /* draw vertical line in negative y direction */
        DrawVLine(g_gdp.regs.penX, 255 - g_gdp.regs.penY, 255 - (g_gdp.regs.penY - g_gdp.regs.deltay));
        g_gdp.contentChanged = 1;
        break;

    case 21: /* draw line in positive x and negative y direction */
        if (g_gdp.regs.ctrl1 & 2)
            pen = fg32;
        else
            pen = bg32;

        DrawLine(g_gdp.regs.penX, 255 - g_gdp.regs.penY, g_gdp.regs.penX + g_gdp.regs.deltax, 255 - (g_gdp.regs.penY - g_gdp.regs.deltay));
        g_gdp.contentChanged = 1;
        g_gdp.regs.penX = g_gdp.regs.penX + g_gdp.regs.deltax;
        g_gdp.regs.penY = g_gdp.regs.penY - g_gdp.regs.deltay;
        break;

    case 22: /* draw horizontal line in negative x direction */
        DrawHLine(g_gdp.regs.penX, 255 - g_gdp.regs.penY, g_gdp.regs.penX - g_gdp.regs.deltax);
        g_gdp.contentChanged = 1;
        break;

    case 23: /* draw line in negative x and positive y direction */
        if (g_gdp.regs.ctrl1 & 2)
            pen = fg32;
        else
            pen = bg32;

        DrawLine(g_gdp.regs.penX, 255 - g_gdp.regs.penY, g_gdp.regs.penX - g_gdp.regs.deltax, 255 - (g_gdp.regs.penY - g_gdp.regs.deltay));
        g_gdp.contentChanged = 1;
        g_gdp.regs.penX = g_gdp.regs.penX - g_gdp.regs.deltax;
        g_gdp.regs.penY = g_gdp.regs.penY - g_gdp.regs.deltay;
        break;

    default: /* unimplemented command, do nothing and return */
        break;
    }
    g_gdp.regs.status = (g_gdp.regs.status | 4);
    return;
}

BYTE gdp64_p71_in()
{
    /* read CTRL1 register of EF9366
     * meanings of the bits:
     * bit 0: pen position, 1=down, 0=up
     * bit 1: pen mode, 1=writing, 0=deleting
     * bit 2: writing on the screen, BLK is always 1, no video signal output
     * bit 3: screen window closed, writing position cannot leave screen
     * bit 4: enable IRQs for lightpens
     * bit 5: enable IRQs for vertical sync
     * bit 6: enables IRQs for GDP64 ready signal
     * bit 7: n/a
     */
    return g_gdp.regs.ctrl1;
}

void gdp64_p71_out(BYTE b)
{
    /* accept values for CTRL1 register of the EF9366 */
    g_gdp.regs.ctrl1 = b;
    return;
}

BYTE gdp64_p72_in()
{
    /* read CTRL2 register of EF9366
     * meanings of the bits:
     * bit 0: type of vectors, LSB
     * bit 1: type of vectors, MSB
     * bit 2: 1=tilted character
     * bit 3: 1=character on vertical axis
     * bit 4: n/a
     * bit 5: n/a
     * bit 6: n/a
     * bit 7: n/a
     */
    return g_gdp.regs.ctrl2;
}

void gdp64_p72_out(BYTE b)
{
    /* accept values for CTRL2 register of the EF9366 */
    g_gdp.regs.ctrl2 = b;
    return;
}

void gdp64_p72_out_word( int b )
{
    /* accept values for CTRL2 and CSIZE registers of the EF9366 */
    /* This is used mainly in a clr.w instruction in GP 6.xx     */
    BYTE msb = (b & 0xFF00) >> 8;
    gdp64_p72_out(msb);
    BYTE lsb = b & 0xFF;
    gdp64_p73_out(lsb);
    return;
}

BYTE gdp64_p73_in()
{
    /* read CSIZE register of EF9366 */
    return g_gdp.regs.csize;
}

void gdp64_p73_out(BYTE b)
{
    /* set CSIZE register of EF9366 */
    g_gdp.regs.csize = b;
}

BYTE gdp64_p75_in()
{
    /* read DELTAX register of EF9366 */
    return g_gdp.regs.deltax;
}

void gdp64_p75_out(BYTE b)
{
    /* set DELTAX register of EF9366 */
//    log_debug("Set DELTAX to %d\n", b);
    g_gdp.regs.deltax = b;
}

BYTE gdp64_p77_in()
{
    /* read DELTAY register of EF9366 */
    return g_gdp.regs.deltay;
}

void gdp64_p77_out(BYTE b)
{
    /* set DELTAY register of EF9366 */
//    log_debug("Set DELTAY to %d\n", b);
    g_gdp.regs.deltay = b;
}

BYTE gdp64_p78_in()
{
    /* read X MSB register of EF9366 */
    return (BYTE)((g_gdp.regs.penX & 0xFF00) >> 8); /* most significant 8 bits */
}

int gdp64_p78_in_word()
{
    /* read penX register of EF9366 as word */
    return g_gdp.regs.penX;
}

void gdp64_p78_out(BYTE b)
{
    /* set X MSB register of EF9366 */
//    log_debug("Set X MSB to %d", b);
    g_gdp.regs.penX = (((int)b) << 8) | (g_gdp.regs.penX & 0xFF);
}

void gdp64_p78_out_word(int b)
{
    g_gdp.regs.penX = b & 0xFFFF;
}

BYTE gdp64_p79_in()
{
    /* read X LSB register of EF9366 */
    return (BYTE)(g_gdp.regs.penX & 0xFF);
}

void gdp64_p79_out(BYTE b)
{
    /* set X LSB register of EF9366 */
//    log_debug("Set X LSB to %d", b);
    g_gdp.regs.penX = ((int)b) | (g_gdp.regs.penX & 0xFF00);
}

BYTE gdp64_p7A_in()
{
    /* read Y MSB register of EF9366 */
    return (BYTE)((g_gdp.regs.penY & 0xFF00) >> 8); /* most significant 8 bits */
}

int gdp64_p7A_in_word()
{
    /* read penY register of EF9366 as word */
    return g_gdp.regs.penY;
}

void gdp64_p7A_out(BYTE b)
{
    /* set Y MSB register of EF9366 */
//    log_debug("Set Y MSB to %d", b);
    g_gdp.regs.penY = (((short)b) << 8) | (g_gdp.regs.penY & 0xFF);
}

void gdp64_p7A_out_word(int b)
{
    g_gdp.regs.penY = b & 0xFFFF;
}

BYTE gdp64_p7B_in()
{
    /* read Y LSB register of EF9366 */
    return (BYTE)(g_gdp.regs.penY & 0xFF);
}

void gdp64_p7B_out(BYTE b)
{
    /* set Y LSB register of EF9366 */
//    log_debug("Set Y LSB to %d", b);
    g_gdp.regs.penY = ((short)b) | (g_gdp.regs.penY & 0xFF00);
}

void gdp64_save_regs()
{
    g_gdp.save_regs.status = g_gdp.regs.status;
    g_gdp.save_regs.ctrl1  = g_gdp.regs.ctrl1;
    g_gdp.save_regs.ctrl2  = g_gdp.regs.ctrl2;
    g_gdp.save_regs.csize  = g_gdp.regs.csize;
    g_gdp.save_regs.penX   = g_gdp.regs.penX;
    g_gdp.save_regs.penY   = g_gdp.regs.penY;
    g_gdp.save_regs.deltax = g_gdp.regs.deltax;
    g_gdp.save_regs.deltay = g_gdp.regs.deltay;
    g_gdp.save_regs.seite  = g_gdp.regs.seite;
}

void gdp64_restore_regs()
{
    g_gdp.regs.status = g_gdp.save_regs.status;
    g_gdp.regs.ctrl1  = g_gdp.save_regs.ctrl1;
    g_gdp.regs.ctrl2  = g_gdp.save_regs.ctrl2;
    g_gdp.regs.csize  = g_gdp.save_regs.csize;
    g_gdp.regs.penX   = g_gdp.save_regs.penX;
    g_gdp.regs.penY   = g_gdp.save_regs.penY;
    g_gdp.regs.deltax = g_gdp.save_regs.deltax;
    g_gdp.regs.deltay = g_gdp.save_regs.deltay;
    g_gdp.regs.seite  = g_gdp.save_regs.seite;

    g_gdp.actualReadPage = (g_gdp.regs.seite & 0x30) >> 4;
    g_gdp.actualWritePage = (g_gdp.regs.seite & 0xC0) >> 6;
}

void gdp64_gui_draw_string(int x, int y, int size, const char * str)
{
    g_gdp.isGuiScreen = true;

    g_gdp.actualWritePage = 4;
    g_gdp.actualReadPage = 4;
    g_gdp.regs.penX = x;
    g_gdp.regs.penY = y;
    g_gdp.regs.csize = size;

    int  i=0;
    while(str[i] != 0x0)
    {
        int cur_x = g_gdp.regs.penX;
        int cur_y = g_gdp.regs.penY;
        char ch = str[i];
        if(ch >= 0x20 && ch <= 0x7F) {
            // Set Erapen
            g_gdp.regs.ctrl1 &= 0b11111101;
            DrawChar(128);              // Draw Block to erase background
            // Set Pen
            g_gdp.regs.penX = cur_x;
            g_gdp.regs.penY = cur_y;
            g_gdp.regs.ctrl1 |= 0b00000010;
            DrawChar(str[i]);           // Draw Character
        }
        if(ch == 0x0a )
        {
            int ysize = (size & 0x0f) * 10;
            g_gdp.regs.penX = x;
            g_gdp.regs.penY -= ysize;
        }
        i++;
    }
}

void gdp64_umrande(int x, int y, int size, int len)
{
    int dx = ((size & 0xF0) >> 4) * len * 6 + 2;
    int dy = (size & 0x0F) * 8 + 3;
    int my_x = x-2;
    int my_y = 255 - (y - 2);
    DrawLine(my_x, my_y, my_x+dx, my_y);
    DrawLine(my_x+dx, my_y, my_x+dx, my_y-dy);
    DrawLine(my_x+dx, my_y-dy, my_x, my_y-dy);
    DrawLine(my_x, my_y-dy, my_x, my_y);
    DrawChar(128);
    g_gui_cursor.isVisible = true;
    g_gui_cursor.isOff = false;
    g_gui_cursor.csize = size;
}

void gdp64_gui_input_string(int x, int y, int size, int len, char * str)
{
    g_gdp.actualWritePage = 4;
    g_gdp.actualReadPage = 4;
    g_gdp.regs.penX = x;
    g_gdp.regs.penY = y;
    g_gdp.regs.csize = size;

    int i = 0;
    if(str != 0)
    {
        int ch = (int) str[0];
        while (ch >= 0x20 && ch <= 0x7F && i < (len-1))
        {
            DrawChar(ch);
            ch = (int) str[++i];
        }
        g_gui_cursor.x = g_gdp.regs.penX;
        g_gui_cursor.y = g_gdp.regs.penY;
    }
    gdp64_umrande (x, y, size, len);
}

void gdp64_gui_blink_cursor()
{
    if( g_gdp.isGuiScreen && g_gui_cursor.isVisible )
    {
        if( g_gui_cursor.counter >= 13) {
            if( ! g_gui_cursor.isOff )
            {
                g_gdp.regs.penX = g_gui_cursor.x;
                g_gdp.regs.penY = g_gui_cursor.y;
                g_gdp.regs.csize = g_gui_cursor.csize;
                g_gdp.regs.ctrl1 &= 0b11111101; // Set Erapen
                DrawChar(128);              // Erase cursor block
                g_gdp.regs.ctrl1 |= 0b00000010;
                g_gui_cursor.isOff = true;
            }
            else {
                // Set Pen
                g_gdp.regs.penX = g_gui_cursor.x;
                g_gdp.regs.penY = g_gui_cursor.y;
                g_gdp.regs.csize = g_gui_cursor.csize;
                DrawChar(128);              // Draw cursor block
                g_gui_cursor.isOff = false;
            }
            g_gui_cursor.counter = 0;
        }
        else
        {
            g_gui_cursor.counter++;
        }

    }
}

int gdp64_init()
{
    int rendererFlags = SDL_RENDERER_SOFTWARE;
//    int rendererFlags = SDL_RENDERER_ACCELERATED;

    g_gdp.xmag = g_config.gdp64XMag;
    g_gdp.ymag = g_config.gdp64YMag;

    g_gdp.window = SDL_CreateWindow("GDP64 graphics output for NKC 68k",
                                    200,
                                    100,
                                    512 * g_gdp.xmag, 256 * g_gdp.ymag,
                                    SDL_SWSURFACE);
    g_gdp.renderer = SDL_CreateRenderer(g_gdp.window, -1, rendererFlags);
    if (g_gdp.window == NULL)
    {
        log_error("Can't set video mode: %s", SDL_GetError());
        exit(1);
    }

    /* create the four pages of the EF9366  plus on GUI page */
    for (int i = 0; i < 5; i++)
    {
//        g_gdp.pages[i] = SDL_CreateRGBSurface(SDL_SWSURFACE, 512 * g_gdp.xmag, 256 * g_gdp.ymag, 32, 0, 0, 0, 0);
        g_gdp.pages[i] = SDL_CreateRGBSurface(SDL_SWSURFACE, 512 * g_gdp.xmag, 256 * g_gdp.ymag, 32, 0, 0, 0, 0);
        if (g_gdp.pages[i] == NULL)
        {
            log_error("Cant't create page %d for EF9366. SDL-Error:%s\n", i, SDL_GetError());
            exit(2);
        }
    }

    return SDL_GetWindowID(g_gdp.window);
}

void gdp64_reset()
{
    g_gdp.regs.status = 0x04;       /* status of the gdp, here initialized with b00000100 */
    g_gdp.regs.ctrl1 = 0x00;        /* CTRL1 register */
    g_gdp.regs.ctrl2 = 0x00;        /* CTRL2 register */
    g_gdp.regs.penX = 0;            /* Pen X and Y position */
    g_gdp.regs.penY = 0;
    g_gdp.regs.csize = 0x11;        /* CSIZE register */
    g_gdp.regs.deltax = 0x00;       /* DELTAX register */
    g_gdp.regs.deltay = 0x00;       /* DELTAY register */
    g_gdp.regs.seite = 0;           /* PAGE register */
    g_gdp.regs.scroll = 0;          /* HARD-SCROLL register */
    g_gdp.actualWritePage = 0;      /* on which page do we write at the moment? */
    g_gdp.actualReadPage = 0;       /* which page is shown at the moment? */
    g_gdp.contentChanged = 0;       /* something new written? */
    g_gdp.isGuiScreen = false;
    clearScreen();
}

void gdp64_clear_screen()
{
    clearScreen();
}

void gdp64_set_vsync(BYTE vs)
{
    if (vs != 0)
    {
        if( g_gdp.isGuiScreen )
            gdp64_gui_blink_cursor();
        g_gdp.regs.status = (g_gdp.regs.status | 2);
        /* now blit actual readed page if something has changed */
        if (g_gdp.contentChanged == 1)
        {
            // Scroll Screen by g_gdp.regs.scroll pixels down
            const unsigned int scroll_value = (unsigned int)(g_gdp.regs.scroll & 0xFE);
            if (scroll_value!=0){
                // Window is 256*3 = 768 in height, y-coordinates are mirrores (0,0 is top-left)
               {
                    // 1. Scroll upper part (scroll...top) down to 0
                    const SDL_Rect src = {.x=0,.y=0,.w=512 * g_gdp.xmag, .h=(256 - scroll_value) * g_gdp.ymag};
                    SDL_Rect dest      = {.x=0,.y=(scroll_value) * g_gdp.ymag,.w=512 * g_gdp.xmag, .h=(256 - scroll_value) * g_gdp.ymag};
                    SDL_BlitSurface(g_gdp.pages[g_gdp.actualReadPage], &src, SDL_GetWindowSurface(g_gdp.window), &dest);
                    //printf("Scroll: %u %u\r\n",scroll_value,g_gdp.ymag);
                    //printf("rect1: y:%u, h:%u -> y:%u, h:%u\r\n",src.y, src.h, dest.y, dest.h);
                    //fflush(stdout);
                }
                {
                    // 2. Scroll lower part (0...scroll) up to top
                    const SDL_Rect src = {.x=0,.y=(256 - scroll_value) * g_gdp.ymag,.w=512 * g_gdp.xmag, .h=scroll_value * g_gdp.ymag};
                    SDL_Rect dest      = {.x=0,.y=0,.w=512 * g_gdp.xmag, .h=scroll_value * g_gdp.ymag};
                    SDL_BlitSurface(g_gdp.pages[g_gdp.actualReadPage], &src, SDL_GetWindowSurface(g_gdp.window), &dest);
                    //printf("rect2: y:%u, h:%u -> y:%u, h:%u\r\n",src.y, src.h, dest.y, dest.h);
                    //fflush(stdout);
                }
            }else{
                SDL_BlitSurface(g_gdp.pages[g_gdp.actualReadPage], NULL, SDL_GetWindowSurface(g_gdp.window), NULL);
            }
            SDL_RenderPresent(g_gdp.renderer);
            g_gdp.contentChanged = 0;
        }
    }
    else
    {
        g_gdp.regs.status = (g_gdp.regs.status & 0xFD);
    }
    return;
}

void gdp64_event(SDL_Event *event)
{
   	if (event->type == SDL_MOUSEMOTION ) {
        Uint32 flags = SDL_GetWindowFlags(g_gdp.window);
        if (( flags & SDL_WINDOW_MOUSE_FOCUS ) == 0)
            SDL_RaiseWindow(g_gdp.window);
    }

    if (event->type == SDL_KEYDOWN)
    {
        /* first evaluate simulation keys */

        if (event->key.keysym.sym == SDLK_F1)
        {
            Uint32 FullscreenFlag = SDL_WINDOW_FULLSCREEN;
            bool IsFullscreen = SDL_GetWindowFlags(g_gdp.window) & FullscreenFlag;
            SDL_SetWindowFullscreen(g_gdp.window, IsFullscreen ? 0 : FullscreenFlag);
            SDL_ShowCursor(IsFullscreen);
        }
    }
}
