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
 * Emulates the UHR interface.
 * The Clock is automatically set from the system clock at each run.
 * If you reset to a different date/time only a difference is calculated
 * and applied to the results. Currently this difference is not persisted 
 * between runs.
 */

#include <stdio.h>
#include <string.h> /* strcat */
#include <stdlib.h> /* strtol */
#include <stdbool.h>
#include "uhr.h"
#include "log.h"

uhr g_uhr;

/*
 * Internal helpers
 */

BYTE_68K toBCD(int value)
{

	BYTE_68K val = (BYTE_68K)value % 100;

	BYTE_68K highDecimal = val / 10;
	BYTE_68K lowDecimal = val % 10;

	return (highDecimal << 4) + lowDecimal;
}

BYTE_68K fromBCD(BYTE_68K value)
{

	return ((value >> 4) * 10 + (value & 0x0F));
}

BYTE_68K rotateRight(BYTE_68K x)
{

	BYTE_68K shifted = x >> 1;
	BYTE_68K rot_bits = x << 7;

	return shifted | rot_bits;
}

BYTE_68K rotateLeft(BYTE_68K x)
{

	BYTE_68K shifted = x << 1;
	BYTE_68K rot_bits = x >> 7;

	return shifted | rot_bits;
}

const char *byte_to_binary(int x)
{
	static char b[9];
	b[0] = '\0';

	int z;
	for (z = 128; z > 0; z >>= 1)
	{
		strcat(b, ((x & z) == z) ? "1" : "0");
	}

	return b;
}

/*
 * UHR functions
 */
BYTE_68K uhr_pFE_in()
{
	log_debug("Reading UHR with bitcounter %d", g_uhr.bitCounter);	
	BYTE_68K ret = 0;
	if((g_uhr.bitCounter % 8) == 0) {
		switch (g_uhr.bitCounter / 8)
		{
		case 7:
			g_uhr.activeByte = g_uhr.hour;
			break;
		case 6:
			g_uhr.activeByte = g_uhr.minute;
			break;
		case 5:
			g_uhr.activeByte = g_uhr.day;
			break;
		case 4:
			g_uhr.activeByte = g_uhr.month;
			break;
		case 3:
			g_uhr.activeByte = g_uhr.year;
			break;
		case 2:
			g_uhr.activeByte = g_uhr.wday;
			break;
		case 1:
			g_uhr.activeByte = g_uhr.second;
			break;
		default:
			printf("Unexpected ERROR\n");
			break; /* code */
		}
	}
	g_uhr.bitCounter--;
	ret = g_uhr.activeByte & 0x01;
	g_uhr.activeByte = rotateRight(g_uhr.activeByte);

	return (ret);
}

void uhr_pFE_out(BYTE_68K b)
{
	bool enable = (b & 0x04) != 0;
	bool pulse = (b & 0x02) != 0;
	log_debug("UHR set %01X Enable=%01X Pulse=%01X Mode =%2d", b, enable, pulse, g_uhr.mode);
	BYTE_68K data = !(b & 0x01);

	if (enable && !g_uhr.enable)
	{ 											// Enable clock chip
		// log_debug("UHR enabled");
		time_t rawtime;
		struct tm *timeinfo;

		time(&rawtime);
		rawtime += g_uhr.diffTime;
		timeinfo = localtime(&rawtime);
		g_uhr.hour = toBCD(timeinfo->tm_hour);
		g_uhr.minute = toBCD(timeinfo->tm_min);
		g_uhr.second = toBCD(timeinfo->tm_sec);
		g_uhr.day = toBCD(timeinfo->tm_mday);
		g_uhr.month = toBCD(timeinfo->tm_mon + 1);
		g_uhr.year = toBCD(timeinfo->tm_year);
		g_uhr.wday = toBCD(timeinfo->tm_wday);
		g_uhr.aktTime = mktime(timeinfo);

		g_uhr.bitCounter = 4; // we expect first to receive 4 bits (3 bit address and r/w mode)
		g_uhr.enable = true;
		return;
	}

	if (!enable && g_uhr.enable)
	{ 									// Disable clock chip
		// log_debug("UHR disabled");
		g_uhr.enable = false;
		g_uhr.mode = -1;
		return;
	}

	if (!g_uhr.taktHigh && pulse)
	{
		g_uhr.taktHigh = true;
		return;
	}
	if (g_uhr.taktHigh && !pulse)
	{									// falling edge
		g_uhr.taktHigh = false;
		// log_debug("UHR falling edge");
	}									// process data

	if (g_uhr.bitCounter == 0) 			// no more bytes to send or receive
		return;

	if (g_uhr.enable && g_uhr.mode != 1)
	{
		log_debug("Writing UHR with bitcounter %d", g_uhr.bitCounter);	
		g_uhr.bitCounter--;
		if (g_uhr.mode == -1)
		{ 								// receiving 4 bits of Addr + Mode
			if ((g_uhr.bitCounter > 0))
				g_uhr.addr = (g_uhr.addr << 1) | data;
			else
			{
				g_uhr.mode = data;
				g_uhr.bitCounter = 7 * 8; // receive/transmit 7 BCD numbers as 8 bits.
				return;
			}
		}
		else
		{ // receiving clock data
			g_uhr.receiveByte = (BYTE_68K)(g_uhr.receiveByte << 1) | data;
			if ((g_uhr.bitCounter % 8) == 0)
			{
				BYTE_68K val = fromBCD(g_uhr.receiveByte);
				switch (g_uhr.bitCounter / 8)
				{
				case 6:
					g_uhr.setTime.tm_hour = val;
					break;
				case 5:
					g_uhr.setTime.tm_min = val;
					break;
				case 4:
					g_uhr.setTime.tm_mday = val;
					break;
				case 3:
					g_uhr.setTime.tm_mon = val-1;
					break;
				case 2:
					if (val < 70)
						val = val + 100;
					g_uhr.setTime.tm_year = val;
					break;
				case 1:
					g_uhr.setTime.tm_wday = val;
					break;
				case 0:
					g_uhr.setTime.tm_sec = val;
					g_uhr.setTime.tm_isdst = -1;
					g_uhr.ndrTime = mktime( &g_uhr.setTime);
					log_debug("Setting time to %d:%d:%d - %d.%d.%d - %d",
						 g_uhr.setTime.tm_hour, g_uhr.setTime.tm_min, g_uhr.setTime.tm_sec,
						 g_uhr.setTime.tm_mday, g_uhr.setTime.tm_mon, g_uhr.setTime.tm_year,
						 g_uhr.setTime.tm_wday);
					g_uhr.ndrTime = mktime( &g_uhr.setTime);
					if(g_uhr.ndrTime == -1) {
						log_error("Invalid time");
						g_uhr.ndrTime = g_uhr.aktTime;
					}
					time_t rawtime;
					g_uhr.diffTime = g_uhr.ndrTime - time(&rawtime);
					break;
				default:
					printf("Unexpected write ERROR\n");
					break;
				}
				g_uhr.receiveByte = 0;
			}
		}
	}

	return; /* no output on key ports */
}

void uhr_reset()
{
	g_uhr.enable = false; 		/* disable clock chip */
	g_uhr.taktHigh = false;
	g_uhr.mode = -1;
	g_uhr.diffTime = 0;
	return;
}
