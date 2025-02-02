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

/**
 * Emulates a Centronics printer interface by writing an ASCII file.
 * Files are written to the home directory of the user or the configured directory.
 */
#include <stdio.h>
#include <unistd.h>
#include "centronics.h"
#include "config.h"
#include "util.h"

cent g_cent;

extern config g_config;

/*
 * Implementation of the centronics interface
 */
BYTE_68K cent_p48_in()
{
  return 0;
}

void cent_p48_out(BYTE_68K data)
{
  g_cent.daten = data;
}

BYTE_68K cent_p49_in()
{
  return g_cent.status;
}

void cent_p49_out(BYTE_68K data)
{
  if((data & 0x01) == 0) {      // Strobe
    if (g_cent.list_file != 0 && g_cent.daten != 0)
    {
      fwrite(&g_cent.daten, 1, 1, g_cent.list_file);
      fflush(g_cent.list_file);
    }
  }
  g_cent.daten = 0;
  g_cent.status = 0;
  return;
}

void cent_reset()
{
  if(g_cent.list_file != NULL)
    fseek(g_cent.list_file, 0, SEEK_SET);
  g_cent.daten = 0;
}

void cent_setFile(const char *filename)
{
    if (g_cent.list_file != 0)
    {
        fclose(g_cent.list_file);
        g_cent.list_file = 0;
    }
    g_cent.list_file = fopen(filename, "wb");
    if( g_cent.list_file == 0 )
    {
        log_warn("Can't open LIST file %s\n", filename);
    }
}