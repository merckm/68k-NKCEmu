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
 * Emulates the EPROM programmer. EPROMs are simulated as files
 * Files are read/written and the location can be configured in the config file.
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include "nkc.h"
#include "log.h"
#include "promer.h"

promer g_promer;

BYTE_68K promer_p80_in()
{
  BYTE_68K ret = 0xFF;
  if(g_promer.read) {
    if (g_promer.prom_file != 0 && g_promer.adr < g_promer.size)
    {
        if (fseek(g_promer.prom_file,g_promer.adr, SEEK_SET) >= 0)
          fread(&ret, 1, 1, g_promer.prom_file);
    }
  }
  return ret;
}

void promer_p80_out(BYTE_68K data)
{
  g_promer.daten = data;
}

BYTE_68K promer_p81_in()
{
  BYTE_68K status = 0x00;
  struct timeval akttime;

  gettimeofday(&akttime,NULL);
  if(timercmp( &akttime, &g_promer.stoptime, >) )
    status = 0x01;            // Busy

  return status;
}

void promer_p81_out(BYTE_68K data)
{
  g_promer.adr = (g_promer.adr & 0xFF00) + data;
}

BYTE_68K promer_p82_in()
{
  // Not allowed function
  BYTE_68K byte = 0x00;
  return byte;
}

void promer_p82_out(BYTE_68K data)
{

  g_promer.adr = (g_promer.adr & 0x00FF) + ((data & 0x1F) << 8);

  if((data & 0x80) != 0)
    g_promer.read = false;
  else
    g_promer.read = true;

  if((data & 0x40) != 0)
    g_promer.led = false;
  else
    g_promer.led = true;

  if((data & 0x20) != 0) {
  if (g_promer.prom_file != 0 && g_promer.adr < g_promer.size && g_promer.read == false)
  {
      if (fseek(g_promer.prom_file,g_promer.adr, SEEK_SET) >= 0)
      {
        BYTE_68K oldData = 0xFF;
        BYTE_68K newData = 0xFF;
        fread(&oldData, 1, 1, g_promer.prom_file);
        fseek(g_promer.prom_file,g_promer.adr, SEEK_SET);
        newData = g_promer.daten & oldData;
        int numWritten = fwrite(&newData, 1, 1, g_promer.prom_file);
        fflush(g_promer.prom_file);
      }
    }

    struct timeval akttime;
    gettimeofday(&akttime,NULL);
//    timeradd(&akttime, &g_promer.progtime, &g_promer.stoptime);
  }
}

void promer_reset()
{
  struct timeval akttime;

  g_promer.led = false;
  g_promer.read = true;
  g_promer.progtime.tv_sec = 0;
  g_promer.progtime.tv_usec = 50000;
  if(g_promer.prom_file != NULL)
    fseek(g_promer.prom_file, 0, SEEK_SET);
  gettimeofday(&akttime,NULL);
//  timersub(&akttime, &g_promer.progtime, &g_promer.stoptime);
}

void promer_setFile(const char *filename)
{
    if (g_promer.prom_file != 0)
    {
        fclose(g_promer.prom_file);
        g_promer.prom_file = 0;
    }
    g_promer.prom_file = fopen(filename, "rb+");
    if( g_promer.prom_file == 0 )
    {
        log_warn("Can't open PROM file %s\n", filename);
    }
    fseek(g_promer.prom_file, 0, SEEK_END);
    g_promer.size = ftell(g_promer.prom_file);
    fseek(g_promer.prom_file, 0, SEEK_SET);   // rewind
}