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

#include <stdio.h>
#include "gui_button.h"
#include "nkc.h"
#include "nkcgui.h"
#include "gdp64.h"

extern gdp64 g_gdp;

gui_button* button_new(int x, int y, int w, int h) {
    gui_button* button = (gui_button*) malloc(sizeof(gui_button));
    if(button != NULL)
    {
        double scale = (512. * g_gdp.xmag) / SCREEN_WIDTH; 

        button->rect.x = rint(x * scale);
        button->rect.y = rint(y * scale);
        button->rect.w = rint(w * scale);
        button->rect.h = rint(h * scale);
    }
    return button;
}

int button_draw(gui_button* button, SDL_Renderer* renderer) {	
    return 0;    
}

bool button_event(gui_button* button, int x, int y) {
    if(button != NULL)
    {
        return x >= button->rect.x && x < button->rect.x + button->rect.w &&
	        y >= button->rect.y && y < button->rect.y + button->rect.h;
    }
    return false;
}