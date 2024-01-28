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

#ifndef HEADER__NKCGUI 
#define HEADER__NKCGUI
#include "nkc.h"
#include "gui_button.h"

#define SCREEN_WIDTH 1066   //900
#define SCREEN_HEIGHT 234   //200

#define GUI_PROM        1
#define GUI_CAS         2
#define GUI_CENT        3
#define GUI_SOUND       4
#define GUI_FLOPPY_A    5
#define GUI_FLOPPY_B    6
#define GUI_JOY_A       7
#define GUI_JOY_B       8

typedef struct {
    SDL_Window      *window;
    SDL_Renderer    *renderer;
    SDL_Texture     *nkc_texture;
    SDL_Texture     *floppy_texture;
    SDL_Texture     *floled_texture;
    SDL_Texture     *red_leds_texture;
    SDL_Texture     *green_leds_texture;
    SDL_Texture     *toggle_off_texture;
    SDL_Texture     *toggle_on_texture;
    SDL_Texture     *push_texture;
    TTF_Font        *font;
    gui_button*     gui_buttons[GUI_NUM_BUTTONS];
    char*           current_path;
    int             active_dialog;
} gui;

#ifdef __cplusplus
extern "C"
{
#endif

    void gui_quit(void);
    int  gui_init(void);
    void gui_draw(void);
    void gui_event(SDL_Event* event);

#ifdef __cplusplus
}
#endif

#endif /* HEADER__NKCGUI */