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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include "nkc.h"
#include "log.h"
#include "util.h"

long nkc_get_diff_micros(struct timeval *t1, struct timeval *t2)
{
    long us = 0;
    if (t2->tv_sec > t1->tv_sec)
    {
        us = (t2->tv_sec - t1->tv_sec) * 1000000 + (t2->tv_usec - t1->tv_usec);
    }
    else
    {
        us = (t2->tv_usec - t1->tv_usec);
    }
    return us;
}

long long nkc_get_diff_nanos(struct timespec* start, struct timespec* end)
{
    long sec = 0;
    long long nsec = 0;

    if ((end->tv_nsec - start->tv_nsec)<0) {
        sec = end->tv_sec - start->tv_sec-1;
        nsec = 1000000000 + end->tv_nsec - start->tv_nsec;
    } else {
        sec = end->tv_sec - start->tv_sec;
        nsec = end->tv_nsec - start->tv_nsec;
    }
    nsec += sec * 1000000000;
    return nsec;
}

void nkc_sleep_nanos(long long ns)
{
    long sleeploops = 0;
    struct timespec start, end;
    clock_gettime( CLOCK_REALTIME, &start);
    long long elapsedNanos = 0;
    while (elapsedNanos < ns)
    {
        sleeploops++;
        clock_gettime( CLOCK_REALTIME, &end);
        elapsedNanos = nkc_get_diff_nanos(&start, &end);
    }
    //log_info("Sleeptime: %d Sleeploops %ld", ns, sleeploops);

}

nkc_array* nkc_arr_new()
{
    nkc_array* array = malloc(sizeof(nkc_array));
    array->data = (void **) malloc( NKC_ARRAY_MIN_SIZE * sizeof(void *));
    array->max_elements = NKC_ARRAY_MIN_SIZE;
    array->num_elements = 0;
    return array;
}

int nkc_arr_append(nkc_array* array, void *data)
{
	if (array == NULL)
        return -1;

    if(array->num_elements >= (array->max_elements -1))
    {
        array->max_elements *= 2;
		array->data = (void **) realloc(array->data,
                        array->max_elements * sizeof(void *));
    }
    array->data[array->num_elements] = data;
	array->num_elements++;

    return 0;
}

int nkc_arr_appendString(nkc_array* array, const char * str)
{
	if (array == NULL)
        return -1;

    char *str_ptr = (char *) malloc(strlen(str) + (1 * sizeof(wchar_t)) );
    strcpy(str_ptr, str);
    return nkc_arr_append(array, str_ptr);
}

void nkc_arr_free(nkc_array* array)
{ 
	if (array == NULL)
        return;

    for( int i = 0; i < array->num_elements; i++)
        free( array->data[i] );
    // free(array->data);
    // array->data = NULL;
    free(array);
}

BYTE nkc_get_ascii(SDL_KeyboardEvent keyEvent)
{
    SDL_Keycode keycode = keyEvent.keysym.sym;
    Uint16 mod = keyEvent.keysym.mod;

    bool isCapsOn = ((mod & KMOD_CAPS) != 0);
    bool isUpper = ((mod & KMOD_LSHIFT) != 0) || ((mod & KMOD_RSHIFT) != 0);
    if (isCapsOn)
        isUpper = !isUpper;

    bool isCtrl = ((mod & KMOD_LCTRL) != 0) || ((mod & KMOD_RCTRL) != 0);
    bool isAlt = (mod & KMOD_LALT) != 0;
    bool isAltGr = (mod & KMOD_RALT) != 0;

    if (isAlt)
        return 0x80;

    BYTE offset = 0x60;
    if (isUpper)
        offset = 0x40;
    if (isCtrl)
        offset = 0x0;

    switch (keycode)
    {
    case SDLK_TAB:
        return 0x09; // Horizontal Tab
    case SDLK_UP:
        return 0x05; // Up -> Ctrl-E
    case SDLK_DOWN:
        return 0x18; // Down -> Ctrl-X
    case SDLK_RIGHT:
        return 0x04; // Right -> Ctrl-D
    case SDLK_LEFT:
        return 0x13; // Left -> Ctrl-S
    case SDLK_HOME:
        return 0x01; // Start of Text
    case SDLK_END:
        return 0x06; // End of Text
    case SDLK_PAGEUP:
        return 0x12; // Page Up -> Ctrl-R
    case SDLK_PAGEDOWN:
        return 0x03; // Page Down -> Ctrl-C
    case SDLK_BACKSPACE:
        return 0x08; // BS
    case SDLK_SPACE:
        return 0x20;
    case SDLK_HASH:
        return (isUpper ? 0x27 : 0x23);
    case SDLK_RETURN:
        return 0x0D; // CR
    case SDLK_ESCAPE:
        return 0x1B; // ESC
    case SDLK_QUOTE:
        return 0x27; // Single quote
    case SDLK_PLUS:
        return (isUpper ? 0x2A : (isAltGr ? 0x7E : 0x2B));
    case SDLK_COMMA:
        return (isUpper ? 0x3B : 0x2C);
    case SDLK_MINUS:
        return (isUpper ? 0x5F : 0x2D);
    case SDLK_PERIOD:
        return (isUpper ? 0x3A : 0x2E);
    case SDLK_SLASH:
        return 0x2F;
    case SDLK_SEMICOLON:
        return 0x3B;
    case SDLK_LESS:
        return (isUpper ? 0x3E : (isAltGr ? 0x7C : 0x3C));
    case SDLK_EQUALS:
        return 0x3B;
    case SDLK_LEFTBRACKET:
        return 0x5B;
    case SDLK_BACKSLASH:
        return 0x5C;
    case SDLK_RIGHTBRACKET:
        return 0x5D;
    case SDLK_CARET:
        return 0x5E;
    case SDLK_BACKQUOTE:
        return 0x60;
//    case SDLK_INSERT:
//        return 0x16;
    case SDLK_DELETE:
        return 0x7F;
    case SDLK_KP_0:
        return 0x30;
    case SDLK_KP_1:
        return 0x31;
    case SDLK_KP_2:
        return 0x32;
    case SDLK_KP_3:
        return 0x33;
    case SDLK_KP_4:
        return 0x34;
    case SDLK_KP_5:
        return 0x35;
    case SDLK_KP_6:
        return 0x36;
    case SDLK_KP_7:
        return 0x37;
    case SDLK_KP_8:
        return 0x38;
    case SDLK_KP_9:
        return 0x39;
    case SDLK_KP_PERIOD:
        return 0x2C; // Comma ???
    case SDLK_KP_DIVIDE:
        return 0x2F;
    case SDLK_KP_MULTIPLY:
        return 0x2A;
    case SDLK_KP_MINUS:
        return 0x2D;
    case SDLK_KP_PLUS:
        return 0x2B;
    case SDLK_KP_ENTER:
        return 0x0D;
    case SDLK_KP_EQUALS:
        return 0x3D;
    case SDLK_0:
        return (isUpper ? 0x3D : (isAltGr ? 0x7D : 0x30));
    case SDLK_1:
        return (isUpper ? 0x21 : 0x31);
    case SDLK_2:
        return (isUpper ? 0x22 : 0x32);
    case SDLK_3:
        return (isUpper ? 0x40 : 0x33);
    case SDLK_4:
        return (isUpper ? 0x24 : 0x34);
    case SDLK_5:
        return (isUpper ? 0x25 : 0x35);
    case SDLK_6:
        return (isUpper ? 0x26 : 0x36);
    case SDLK_7:
        return (isUpper ? 0x2F : (isAltGr ? 0x7B : 0x37));
    case SDLK_8:
        return (isUpper ? 0x28 : (isAltGr ? 0x5B : 0x38));
    case SDLK_9:
        return (isUpper ? 0x29 : (isAltGr ? 0x5D : 0x39));
    case SDLK_a:
        return (0x01 + offset);
    case SDLK_b:
        return (0x02 + offset);
    case SDLK_c:
        return (0x03 + offset);
    case SDLK_d:
        return (0x04 + offset);
    case SDLK_e:
        return (0x05 + offset);
    case SDLK_f:
        return (0x06 + offset);
    case SDLK_g:
        return (0x07 + offset);
    case SDLK_h:
        return (0x08 + offset);
    case SDLK_i:
        return (0x09 + offset);
    case SDLK_j:
        return (0x0A + offset);
    case SDLK_k:
        return (0x0B + offset);
    case SDLK_l:
        return (0x0C + offset);
    case SDLK_m:
        return (0x0D + offset);
    case SDLK_n:
        return (0x0E + offset);
    case SDLK_o:
        return (0x0F + offset);
    case SDLK_p:
        return (0x10 + offset);
    case SDLK_q:
        return (0x11 + offset);
    case SDLK_r:
        return (0x12 + offset);
    case SDLK_s:
        return (0x13 + offset);
    case SDLK_t:
        return (0x14 + offset);
    case SDLK_u:
        return (0x15 + offset);
    case SDLK_v:
        return (0x16 + offset);
    case SDLK_w:
        return (0x17 + offset);
    case SDLK_x:
        return (0x18 + offset);
    case SDLK_y:
        return (0x19 + offset);
    case SDLK_z:
        return (0x1A + offset);
    case 180:
        return (isUpper ? 0x60 : 0x27); // Single Qoutes
    case 223:
        return (isUpper ? 0x3F : 0x7E); // Eszet
    case 228:
        return (isUpper ? 0x5B : 0x7B); // Ä
    case 246:
        return (isUpper ? 0x5C : 0x7C); // Ö
    case 252:
        return (isUpper ? 0x5D : 0x7D); // Ü
    }

    return 0x80;
}

char * nkc_normalize_path(char * base_path, const char * rel_path)
{
    char* absolut_path = NULL;
    char* new_base_path = NULL;
    int rel_path_start = 0;
    if( (strlen(rel_path) > 0) && (rel_path[0] != '.'))
    {
        // If rel_path does not start with . or .. it is an absolut path
        absolut_path = malloc(strlen(rel_path) + (1 * sizeof(wchar_t)) );
        strcpy(absolut_path, rel_path);
        return absolut_path;
    }
    if( (strlen(rel_path) == 1) && (rel_path[0] == '.'))
    {
        // If rel_path is just a dot
        absolut_path = malloc(strlen(base_path) + (1 * sizeof(wchar_t)) );
        strcpy(absolut_path, base_path);
        return absolut_path;
    }
    if( (strlen(rel_path) > 1) && (rel_path[1] == '.'))
    {
        // If rel_path starts with '../' remove last path element from base_path 
        // and ignore first 3 characters 
        new_base_path = nkc_remove_last(base_path);
        rel_path_start = 3;
    }
    new_base_path = strdup(base_path);
    if( (strlen(rel_path) > 1) && 
        ( (rel_path[1] == '/') || (rel_path[1] == '\\') ) )
    {
        // If rel_path starts with './' ignore first 2 characters 
        rel_path_start = 2;
    }

    int len_rel_path = strlen(rel_path + rel_path_start);

    char* file_rel_path = malloc(len_rel_path + (2 * sizeof(wchar_t)));
    file_rel_path[0] = PATH_SEPARATOR;
    file_rel_path[len_rel_path+1] = 0;
    strncpy(file_rel_path+1, rel_path+rel_path_start, len_rel_path);
    absolut_path = malloc(strlen(new_base_path) + len_rel_path + (1 * sizeof(wchar_t)));
    strcpy(absolut_path, new_base_path); 
    absolut_path = strcat(absolut_path, file_rel_path);

    log_debug("Normalized Path:  %s",absolut_path);
    if(new_base_path != NULL)
        free(new_base_path);
    if(file_rel_path != NULL)
        free(file_rel_path);
    return absolut_path;
}

char* nkc_append_file(char * path, const char * file_name)
{
    char* new_path;
    // append file_name ti  to path string
    new_path = malloc(strlen(path) + strlen(file_name) + (2 * sizeof(char)) );
    strcpy(new_path, path);
    new_path[strlen(path)] = PATH_SEPARATOR;
    strcpy(new_path+strlen(path)+1, file_name);
    return new_path;
}

char* nkc_append_path(char * path, const char * directory)
{
    char* new_path;
    if( directory == NULL || strlen(directory) == 0)
        return path;                    // TODO: hoew to free, maybe make a copy
    if( directory[0] == '.')
    {
        if( strlen(directory) == 1 )      // do nothing for "."
        {
            char * new_path = malloc(strlen(path) +1);
            strcpy(new_path, path);
            return new_path;
        }
        if( strlen(directory) == 2 && directory[1] == '.')
        {
            // If ".." remove last directory path element 
            char * new_path = nkc_remove_last(path);
            return new_path;
        }
    }
    // append directory to path string
    new_path = malloc(strlen(path) + strlen(directory) + (2 * sizeof(char)) );
    strcpy(new_path, path);
    new_path[strlen(path)] = PATH_SEPARATOR;
    strcpy(new_path+strlen(path)+1, directory);
    return new_path;
}

char* nkc_remove_last(char * path)
{
    char* last_path_separator = strrchr(path, '/');
    if (last_path_separator == NULL)
        last_path_separator = strrchr(path, '\\');

    int len_rel_path;    
    if (last_path_separator == NULL)
        len_rel_path = 0;
    else
        len_rel_path = last_path_separator - path;

    char* new_path;
    new_path = malloc(len_rel_path + (1 * sizeof(wchar_t)));
    new_path[len_rel_path] = 0x0;
    strncpy(new_path, path, len_rel_path);
    return new_path;
}

char* nkc_get_filename(const char * path)
{
    char* last_path_separator = strrchr(path, '/');
    if (last_path_separator == NULL)
        last_path_separator = strrchr(path, '\\');

    if (last_path_separator == NULL)
        return (char *) path;

    return last_path_separator+sizeof(char);
}

void* my_malloc(size_t size, const char *file, int line, const char *func)
{

    void *p = malloc(size);
    printf ("Allocated = %s, %d, %s, %p[%llu]\n", file, line, func, p, size);

    return p;
}