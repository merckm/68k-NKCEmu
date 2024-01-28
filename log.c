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
#include <time.h>
#include <string.h>
#include <stdarg.h>
#include "log.h"

void log_format(const char *tag, const char *message, va_list args)
{
    time_t now;
    time(&now);
    char *date = ctime(&now);
    date[strlen(date) - 1] = '\0';
    fprintf(stderr, "%s [%s] ", date, tag);
    vfprintf(stderr, message, args);
    fprintf(stderr, "\n");
}

void log_error(const char *message, ...)
{
    if (LOG_ERROR)
    {
        va_list args;
        va_start(args, message);
        log_format("ERROR", message, args);
        va_end(args);
    }
}

void log_warn(const char *message, ...)
{
    if (LOG_WARN)
    {
        va_list args;
        va_start(args, message);
        log_format("WARNING", message, args);
        va_end(args);
    }
}

void log_info(const char *message, ...)
{
    if (LOG_INFO)
    {
        va_list args;
        va_start(args, message);
        log_format("INFO", message, args);
        va_end(args);
    }
}

void log_debug(const char *message, ...)
{
    if (LOG_DEBUG)
    {
        va_list args;
        va_start(args, message);
        log_format("DEBUG", message, args);
        va_end(args);
    }
}