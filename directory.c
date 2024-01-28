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

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include <unistd.h>

#include "nkcgui.h"
#include "gui_file.h"
#include "directory.h"
#include "log.h"

extern gui g_gui;
extern file_status g_file_stat;

nkc_directory* dir_listfiles(const char * path)
{
    nkc_directory* nkc_dir = malloc(sizeof(nkc_directory));
    nkc_dir->dir_entries = nkc_arr_new();
    nkc_dir->file_entries = nkc_arr_new();
    nkc_dir->active_dir = strdup(path);

    log_debug("Active Path:  %s",nkc_dir->active_dir);

    chdir(nkc_dir->active_dir);
    DIR* dir = opendir(".");
    struct dirent* entry;
    while (true)
    {
        bool is_dir = false;
        bool is_file = false;
        struct stat file_stat;
        entry = readdir(dir);
        if(entry == NULL)
            break;
        stat(entry->d_name, &file_stat);
        is_dir  = S_ISDIR(file_stat.st_mode);
        is_file = S_ISREG(file_stat.st_mode);
        if( is_dir ) {
//            log_debug("Dir  %s",entry->d_name);
            nkc_arr_appendString(nkc_dir->dir_entries, entry->d_name);
        }
        if( is_file ) {
//            log_debug("File %s - %ld",entry->d_name, file_stat.st_size);
            char *search_str = entry->d_name;
            char *result = NULL;
            for (;;) {
                char *p = strstr(search_str, g_file_stat.filter);
                if (p == NULL)
                    break;
                result = p;
                search_str = p + 1;
            }
            ptrdiff_t index = result - entry->d_name;
            if( index == strlen(entry->d_name) - strlen(g_file_stat.filter) )
                nkc_arr_appendString(nkc_dir->file_entries, entry->d_name);
        }
    }
    closedir(dir);
    chdir(g_gui.current_path);
    return nkc_dir;
}

void dir_free(nkc_directory* dir)
{
    nkc_arr_free( dir->dir_entries );
    nkc_arr_free( dir->file_entries );
    free(dir->active_dir);
    free(dir);
}