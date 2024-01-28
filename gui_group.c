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
#include "gui_group.h"
#include "nkc.h"
#include "nkcgui.h"
#include "config.h"
#include "gdp64.h"
#include "log.h"

extern gdp64 g_gdp;
extern gui g_gui;
extern gdp64_gui_cursor g_gui_cursor;
group_status g_group_stat;

void draw_centered2(const char *str, int y, int size)
{
    int sizex = size & 0x0F;
    int ch_capacity = rint(512. / (6 * sizex));
    int x = (ch_capacity - strlen(str)) / 2;
    x *= 6 * sizex;
    gdp64_gui_draw_string(x, y, size, str);
}

void group_new(nkc_array* arr, const char * title) {
    g_group_stat.title = title;
    g_group_stat.active_array = arr;
    g_group_stat.start_arr_entry = 0;
    g_group_stat.input = (char *) malloc( INPUT_MAX_LENGTH );
    g_group_stat.input[0] = 0x0;
    g_gdp.actualReadPage = 4;
    g_gdp.actualWritePage = 4;

    group_display(arr);
}

void group_display(nkc_array* arr) {
    gdp64_clear_screen();
    draw_centered2(g_group_stat.title, 220, 0x33);
    char str[32*7];
    str[0] = 0x0;

    int i = 0;
    while (i<arr->num_elements && i < 6)
    {
        char ch = '1' + i;
        if( i< (arr->num_elements -  g_group_stat.start_arr_entry) )
            sprintf(str,"%s\n%c = %.32s",str, ch,(char *) arr->data[i+  g_group_stat.start_arr_entry]);
        else
            sprintf(str,"%s\n",str);
        i++;
    }
    if ( i < arr->num_elements )
        sprintf(str,"%s\n\nW = weiter",str);
    if ( g_group_stat.start_arr_entry != 0 )
        sprintf(str,"%s\nZ = zurück",str);
    gdp64_gui_draw_string(50, 180, 0x22, str);

    gdp64_gui_input_string(50, 10, 0x33, 2, g_group_stat.input );
    gdp64_gui_draw_string(400, 1, 0x11, "Esc=Abbruch");
    SDL_RaiseWindow(g_gdp.window);
}

void group_select_event(int key) {

    if( key == 0x1B )                   // Escape key exits dialog
    {
        g_group_stat.active_array = NULL;

        free(g_group_stat.input);
        g_group_stat.input = NULL;
        g_gui.active_dialog = 0;
        gdp64_restore_regs();
        g_gdp.isGuiScreen = false;
        SDL_RaiseWindow(g_gdp.window);
        return;
    }    

    if( key == 0xa || key == 0xd )
    {
        if( g_group_stat.input[0] == 'W' )
        {
            if( g_group_stat.active_array->num_elements > ( g_group_stat.start_arr_entry + 6))
                g_group_stat.start_arr_entry += 6;
            group_display(g_group_stat.active_array);
            return;
        }
        if( g_group_stat.input[0] == 'Z' )
        {
            int diff = 6;
            if ( g_group_stat.start_arr_entry > 6 )
                g_group_stat.start_arr_entry -= 6;
            else
                g_group_stat.start_arr_entry = 0;
            group_display(g_group_stat.active_array);
            return;
        }
        if( g_group_stat.input[0] >= '1' && g_group_stat.input[0] <= '6')
        {
            int index = (g_group_stat.input[0] - '1') +
                g_group_stat.start_arr_entry;
            g_group_stat.input[0] = 0;
            if( ! (index >= g_group_stat.active_array->num_elements ) )
            {
                char* selected_element = g_group_stat.active_array->data[index];
                log_debug("Selected element %s",selected_element);
                switch (g_gui.active_dialog)
                {
                    case GUI_SOUND:
                        setAudioDevice(selected_element);
                        break;
                    case GUI_JOY_A:
                        setJoystickA(selected_element);
                        break;
                    case GUI_JOY_B:
                        setJoystickB(selected_element);
                        break;
                    default:
                        break;
                }
                g_group_stat.active_array = NULL;   // already freed in sound.c during reset
                free(g_group_stat.input);
                g_group_stat.input = NULL;
                g_gui.active_dialog = 0;
                gdp64_restore_regs();
                g_gdp.isGuiScreen = false;
                SDL_RaiseWindow(g_gdp.window);
                return;
            }
            else
            {
               group_display(g_group_stat.active_array);
               return;
            }
        }
        group_display(g_group_stat.active_array);
        return;
    }
    else
    {
        int xSize = (g_gdp.regs.csize & 0xF0) >> 4;
        if( strlen(g_group_stat.input) == 0 && key >= 0x20 && key <= 0x7F )
        {
            g_group_stat.input[0] = key;
            g_group_stat.input[1] = 0x0;
            g_gui_cursor.x += 6 * xSize;
            g_gdp.regs.penX += 6 * xSize; // char width + 1
            group_display(g_group_stat.active_array);
        }
        else                        // We can handle baxckspace like any other char
                                    // to delete previousd entered chatacter
        {
            g_group_stat.input[0] = 0x0;
            // Now correct penX and penY
            if (xSize == 0)
                xSize = 16;

            if (g_gdp.regs.ctrl2 & 8)
            {
                g_gui_cursor.y -= 6 * xSize;
                g_gdp.regs.penY -= 6 * xSize;
            }
            else
            {
                g_gui_cursor.x -= 6 * xSize;
                g_gdp.regs.penX -= 6 * xSize; // char width + 1
            }

            group_display(g_group_stat.active_array);
        }
    }
}