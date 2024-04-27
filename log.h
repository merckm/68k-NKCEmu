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

#ifndef LOG_H 
#define LOG_H

//#define LOGLEVEL_INFO
#define LOGLEVEL_DEBUG

#ifdef LOGLEVEL_DEBUG
#define LOG_ERROR 1
#define LOG_WARN 1
#define LOG_INFO 1
#define LOG_DEBUG 1
#endif

#ifdef LOGLEVEL_INFO
#define LOG_ERROR 1
#define LOG_WARN 1
#define LOG_INFO 1
#define LOG_DEBUG 0
#endif

#ifdef LOGLEVEL_WARN
#define LOG_ERROR 1
#define LOG_WARN 1
#define LOG_INFO 0
#define LOG_DEBUG 0
#endif

#ifdef LOGLEVEL_ERROR
#define LOG_ERROR 1
#define LOG_WARN 0
#define LOG_INFO 0
#define LOG_DEBUG 0
#endif

#ifdef LOGLEVEL_NONE
#define LOG_ERROR 0
#define LOG_WARN 0
#define LOG_INFO 0
#define LOG_DEBUG 0
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    void log_error(const char* message, ...);
    void log_warn(const char* message, ...);
    void log_info(const char* message, ...);
    void log_debug(const char* message, ...);

#ifdef __cplusplus
}
#endif

#endif