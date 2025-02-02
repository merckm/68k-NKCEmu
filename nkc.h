/* ======================================================================== */
/* ========================= LICENSING & COPYRIGHT ======================== */
/* ======================================================================== */
/*
 *                            6 8 K - N K C E M U
 *
 * A Simulator for the NDR-Klein-Computer 68K version.
 *
 *   Copyright (C) 2023,2024 by Martin Merck
 *   martin.merck@gmx.de
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef HEADER__NKC
#define HEADER__NKC

#if defined(_MSC_VER)
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#elif defined(__APPLE__)
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#endif

#include <stdio.h>
#include <stdbool.h>

#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif

typedef unsigned int LONG_68K; /* 32 bit unsigned */
typedef unsigned short WORD_68K; /* 16 bit unsigned */
typedef unsigned char BYTE_68K;  /* 8 bit unsigned */

#endif /* HEADER__NKC */