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
 * Emulates a CAS interface with a Motorla 6850 as the serial converter.
 * Files are written to the home directory of the user or the configured directory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "log.h"
#include "cas.h"

cas g_cas;

BYTE transmitter = 0x02;
BYTE receiver = 0x01;

BYTE cas_pCA_in()
{
    return transmitter | receiver; // Allways data ready and ready to transmit
}

void cas_pCA_out(BYTE data)
{
    // Ignoring as we currently do not support interrupts
}

BYTE cas_pCB_in()
{
    BYTE byte = 0xff;
    if (g_cas.cas_file != 0)
    {
        fread(&byte, 1, 1, g_cas.cas_file);
        g_cas.pos = 0;
    }
    return byte;
}

void cas_pCB_out(BYTE data)
{
    if (g_cas.cas_file != 0)
    {
        fwrite(&data, 1, 1, g_cas.cas_file);
        g_cas.pos++;
    }
}

void cas_reset()
{
    if(g_cas.cas_file != NULL)
        fseek(g_cas.cas_file, 0, SEEK_SET);
    g_cas.pos = 0;
}

void cas_setFile(const char *filename)
{
    if(g_cas.num_recordings != 0)
        cas_freeRecordings();

    log_info("Opening CAS file %s", filename);
    if (g_cas.cas_file != 0)
    {
        fclose(g_cas.cas_file);
        g_cas.cas_file = 0;
    }
    g_cas.cas_file = fopen(filename, "rb+");
    if( g_cas.cas_file == 0 )
    {
        log_warn("Can't open CAS file %s", filename);
        return;
    }
    cas_findRecordings();
}

void cas_findRecordings()
{
    char recordingName[100];
    long fillerBytes = 0;
    long curPos = ftell(g_cas.cas_file);
    bool controlSequence = false;
    fseek(g_cas.cas_file, 0L, SEEK_SET);
    while( ! feof(g_cas.cas_file))
    {
        int ch = fgetc(g_cas.cas_file);
        if(ch == 0xFF)
            fillerBytes++;
        if(ch == 0x00 && fillerBytes > 10)
            controlSequence = true;
        if(ch == 0x2F)
        {
            int i = 0;
            while ( i<100 && ch != 0x0D) {
                if(feof(g_cas.cas_file))
                    log_error("Unexpected end of CAS-File. File seems corrupted!\n");
                ch = (char) fgetc(g_cas.cas_file);
                recordingName[i] = ch;

                if( i == 99 ) {
                    log_info("Recording name was truncated to 99 charactrers!\n");
                    ch = 0x0D;
                }
                i++;
            }
            if ( ch == 0x0D) {
                recordingName[i-1] = 0x00;
                cas_recording* recording = (cas_recording*) malloc(sizeof(cas_recording));
                size_t name_len = strlen(recordingName) + 1;     // Add 1 for terminating \0 character
                recording->recording_name = malloc(name_len);
                if( fillerBytes > 40)                            // Ignore possible filler bytes from previous recording
                    fillerBytes = 40;
                recording->start_pos = ftell(g_cas.cas_file) - name_len - 2 - fillerBytes;  // 2 for identifying bytes
                memcpy(recording->recording_name, recordingName, name_len);
                if(g_cas.num_recordings < 100)
                {
                    g_cas.recordings[g_cas.num_recordings] = recording;
                    log_info( "Recording : %s-%d",
                                g_cas.recordings[g_cas.num_recordings]->recording_name,
                                g_cas.recordings[g_cas.num_recordings]->start_pos );
                    g_cas.num_recordings++;
                } else{
                    log_warn("Ignoring recording %s as maximum for tape already stored!", recordingName);
                }
            }
            fillerBytes = 0;
            controlSequence = false;
        }
        else
        { 
            if (ch != 0xFF && ch != 0x00 && ch != 0x27 )
            {
                fillerBytes = 0;
                controlSequence = false;
            }
        }

    }

    fseek(g_cas.cas_file, curPos, SEEK_SET);
    return;
}

void cas_freeRecordings()
{
    for( int i=0; i < g_cas.num_recordings; i++ )
    {
        free(g_cas.recordings[i]->recording_name);
        free(g_cas.recordings[i]);
        g_cas.recordings[i] = NULL;
    }
    g_cas.num_recordings = 0;
}