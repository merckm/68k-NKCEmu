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

#ifndef HEADER__UTIL
#define HEADER__UTIL
#include <stdlib.h>
#include <time.h>
#include "nkc.h"

#if defined(_WIN32) || defined(_WIN64)
    #define PATH_SEPARATOR '\\'
#else
    #define PATH_SEPARATOR '/'
#endif

#define NKC_ARRAY_MIN_SIZE 10

typedef struct {
	int max_elements;
	int num_elements;
    void** data;
} nkc_array;

#ifdef __cplusplus
extern "C"
{
#endif

    long nkc_get_diff_micros(struct timeval *t1, struct timeval *t2);
    long long nkc_get_diff_nanos(struct timespec* start, struct timespec* end);
    void nkc_sleep_nanos(long long ns);

	nkc_array* nkc_arr_new();
    int nkc_arr_append(nkc_array* array, void* data);
    int nkc_arr_appendString(nkc_array* array, const char * str);
    void nkc_arr_free(nkc_array* array);
    BYTE nkc_get_ascii(SDL_KeyboardEvent keyEvent);
    char* nkc_normalize_path(char * base_path, const char * rel_path);
    char* nkc_append_file(char * path, const char * file_name);
    char* nkc_append_path(char * path, const char * directory);
    char* nkc_remove_last(char * path);
    char* nkc_get_filename(const char * path);
    void* my_malloc(size_t size, const char *file, int line, const char *func);

#ifdef __cplusplus
}
#endif

// #define malloc(X) my_malloc( X, __FILE__, __LINE__, __FUNCTION__)

#endif /* HEADER__UTIL */