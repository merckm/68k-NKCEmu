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
#include "nkc.h"
#include "gui_file.h"
#include "nkcgui.h"
#include "log.h"
#include "gdp64.h"
#include "cas.h"
#include "centronics.h"
#include "promer.h"
#include "68k-nkcemu.h"
#include "config.h"
#include "util.h"

extern gdp64 g_gdp;
extern gui g_gui;
extern gdp64_gui_cursor g_gui_cursor;
file_status g_file_stat;

void draw_centered(const char *str, int y, int size)
{
    int sizex = size & 0x0F;
    int ch_capacity = rint(512. / (6 * sizex));
    int x = (ch_capacity - strlen(str)) / 2;
    x *= 6 * sizex;
    gdp64_gui_draw_string(x, y, size, str);
}

void file_new(nkc_directory* dir, const char * title) {
    g_file_stat.title = title;
    g_file_stat.active_dir = dir;
    g_file_stat.start_dir_entry = 0;
    g_file_stat.start_file_entry = 0;
    g_file_stat.input = (char *) malloc( INPUT_MAX_LENGTH );
    g_file_stat.input[0] = 0x0;

    g_gdp.actualReadPage = 4;
    g_gdp.actualWritePage = 4;
    file_display(dir);
}

void file_display(nkc_directory* dir) {
    gdp64_clear_screen();
    draw_centered(g_file_stat.title, 220, 0x33);
    char str[12*40];
    sprintf(str,"Verzeichnis : %.60s",dir->active_dir);
    gdp64_gui_draw_string(40, 205, 0x11, str);
    gdp64_gui_draw_string(40, 190, 0x11, "---- Verzeichnis ----               ----    Datei    ----              ");
    str[0] = 0x0;
    int i = 0;
    while (i<dir->dir_entries->num_elements && i < 12)
    {
        char ch = 'a' + i;
        if( i< (dir->dir_entries->num_elements - g_file_stat.start_dir_entry) )
            sprintf(str,"%s\n%c = %.31s",str, ch,(char *) dir->dir_entries->data[i+ g_file_stat.start_dir_entry]);
        else
            sprintf(str,"%s\n",str);
        i++;
    }
    if ( (i + g_file_stat.start_dir_entry) < dir->dir_entries->num_elements )
        sprintf(str,"%s\n\nw = weiter",str);
    if ( g_file_stat.start_dir_entry != 0 )
        sprintf(str,"%s\nz = zurueck",str);
    gdp64_gui_draw_string(40, 190, 0x11, str);

    str[0] = 0x0;
    i = 0;
    while (i<dir->file_entries->num_elements && i < 12)
    {
        char ch = 'A' + i;
        sprintf(str,"%s\n%c = %.31s",str, ch, (char *) dir->file_entries->data[i+ g_file_stat.start_file_entry]);
        i++;
    }
    if ( i < dir->file_entries->num_elements )
        sprintf(str,"%s\n\nW = weiter",str);
    if ( g_file_stat.start_file_entry != 0 )
        sprintf(str,"%s\nZ = zurück",str);
    gdp64_gui_draw_string(250, 190, 0x11, str);

    gdp64_gui_input_string(50, 10, 0x33, 2, g_file_stat.input );
    gdp64_gui_draw_string(400, 1, 0x11, "Esc=Abbruch");
    SDL_RaiseWindow(g_gdp.window);
}

void file_select_event(int key) {
    if( key == 0x1B )                   // Escape key exits dialog
    {
        dir_free(g_file_stat.active_dir);

        g_gui.active_dialog = 0;
        gdp64_restore_regs();
        g_gdp.isGuiScreen = false;
        SDL_RaiseWindow(g_gdp.window);
        return;
    }    
    if( key == 0xa || key == 0xd )
    {
        if( g_file_stat.input[0] == 'w' )
        {
            if( g_file_stat.active_dir->dir_entries->num_elements > ( g_file_stat.start_dir_entry + 12))
                g_file_stat.start_dir_entry += 12;
            g_file_stat.input[0] = 0;
            file_display(g_file_stat.active_dir);
            return;
        }
        if( g_file_stat.input[0] == 'z' )
        {
            if ( g_file_stat.start_dir_entry > 12 )
                g_file_stat.start_dir_entry -= 12;
            else
                g_file_stat.start_dir_entry = 0;
            g_file_stat.input[0] = 0;
            file_display(g_file_stat.active_dir);
            return;
        }
        if( g_file_stat.input[0] >= 'a' && g_file_stat.input[0] <= 'l')
        {
            int index = (g_file_stat.input[0] - 'a') +
                g_file_stat.start_dir_entry;
            g_file_stat.input[0] = 0;
            if( index < g_file_stat.active_dir->dir_entries->num_elements )
            {
                char* selected_dir = g_file_stat.active_dir->dir_entries->data[index];
                log_debug("Selected dir %s",selected_dir);
                char* new_path = nkc_append_path( g_file_stat.active_dir->active_dir, selected_dir);
                dir_free(g_file_stat.active_dir);
                nkc_directory* dir = dir_listfiles(new_path);
                free(new_path);
                file_new(dir, g_file_stat.title);
                return;
            }
            else {
                file_display(g_file_stat.active_dir);
                return;
            }
        }
        if( g_file_stat.input[0] == 'W' )
        {
            if( g_file_stat.active_dir->file_entries->num_elements > ( g_file_stat.start_file_entry + 12))
                g_file_stat.start_file_entry += 12;
            g_file_stat.input[0] = 0;
            file_display(g_file_stat.active_dir);
            return;
        }
        if( g_file_stat.input[0] == 'Z' )
        {
            if ( g_file_stat.start_file_entry > 12 )
                g_file_stat.start_file_entry -= 12;
            else
                g_file_stat.start_file_entry = 0;
            g_file_stat.input[0] = 0;
            file_display(g_file_stat.active_dir);
            return;
        }
        if( g_file_stat.input[0] >= 'A' && g_file_stat.input[0] <= 'L')
        {
            int index = (g_file_stat.input[0] - 'A') +
                g_file_stat.start_file_entry;
            g_file_stat.input[0] = 0;
            if( ! (index >= g_file_stat.active_dir->file_entries->num_elements ) )
            {
                char* selected_file = g_file_stat.active_dir->file_entries->data[index];
                char* fullFilePath = nkc_append_file( g_file_stat.active_dir->active_dir,
                            selected_file);
                log_debug("Selected file %s %s",selected_file,fullFilePath);
                switch (g_gui.active_dialog)
                {
                    case GUI_CAS:
                        setCasFileName(fullFilePath);
                        break;
                    case GUI_PROM:
                        setPromFileName(fullFilePath);
                        break;
                    case GUI_CENT:
                        setListFileName(fullFilePath);
                        break;
                    case GUI_FLOPPY_A:
                        setFloppyA(fullFilePath);
                        break;
                    case GUI_FLOPPY_B:
                        setFloppyB(fullFilePath);
                        break;
                    default:
                        break;
                }
                free(fullFilePath);
                dir_free(g_file_stat.active_dir);

                g_gui.active_dialog = 0;
                gdp64_restore_regs();
                g_gdp.isGuiScreen = false;
                SDL_RaiseWindow(g_gdp.window);
                return;
            }
            else
            {
               file_display(g_file_stat.active_dir);
               return;
            }
        }
        g_file_stat.input[0] = 0;
        file_display(g_file_stat.active_dir);
        return;
    }
    else
    {
        int xSize = (g_gdp.regs.csize & 0xF0) >> 4;
        if( strlen(g_file_stat.input) == 0 && key >= 0x20 && key <= 0x7F )
        {
            g_file_stat.input[0] = key;
            g_file_stat.input[1] = 0x0;
            g_gui_cursor.x += 6 * xSize;
            g_gdp.regs.penX += 6 * xSize; // char width + 1
            file_display(g_file_stat.active_dir);
        }
        else
        {
            g_file_stat.input[0] = 0x0;
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

            file_display(g_file_stat.active_dir);
        }
    }
}