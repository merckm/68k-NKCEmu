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

#ifndef HEADER__GUI_BUTTON
#define HEADER__GUI_BUTTON

#include "nkc.h"

#define BUTTON_PROM     0
#define BUTTON_CAS      1
#define BUTTON_CENT     2
#define BUTTON_SOUND    3
#define BUTTON_FLOPPY_A 4
#define BUTTON_FLOPPY_B 5
#define BUTTON_JOY_A    6
#define BUTTON_JOY_B    7
#define BUTTON_PASTE   8
#define BUTTON_RESET    9
#define BUTTON_TURBO    10
#define BUTTON_INT      11
#define BUTTON_NMI      12
#define GUI_NUM_BUTTONS 13

typedef struct {
  const char* text;
  TTF_Font* font;
  SDL_Rect rect;
  SDL_Color color;
} gui_button;

#ifdef __cplusplus
extern "C"
{
#endif

  gui_button* button_new(int x,int y,int w,int h);
  int button_draw(gui_button* button, SDL_Renderer* renderer);
  bool button_event(gui_button* button, int x, int y);

#ifdef __cplusplus
}
#endif

#endif /* HEADER__GUI_BUTTON */