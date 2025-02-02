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
 * Emulates a FLO2 interface with up to 4 Drives.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include "flo2.h"
#include "crc.h"
#include "config.h"
#include "log.h"

// BYTE_68K interrupt = 0x06;

flo2 g_flo2;
extern config g_config;

FILE* TRACK_FILE = 0;

void runVerify() {
	BYTE_68K drive = g_flo2.drive & 0x0F;
	BYTE_68K driveType = g_flo2.drive & 0b00110000;
	switch(drive)
	{
	case 1:
    	if( driveType != DRIVE_MINI_DD )
			g_flo2.status |= STATUS_I_CRC_ERR;
    	break;
	case 2:
    	if( driveType != DRIVE_MINI_DD )
			g_flo2.status |= STATUS_I_CRC_ERR;
    	break;
	case 4:
    	if( driveType != DRIVE_MAXI_DD )
			g_flo2.status |= STATUS_I_CRC_ERR;
    	break;
	case 8:
    	if( driveType != DRIVE_MAXI_DD )
			g_flo2.status |= STATUS_I_CRC_ERR;
    	break;
	default:
    	log_warn("Unsupported drive %d", drive);
    	break;
	}
}

void dumpSector()
{
	for (int i=0; i< 64; i++)
	{
		for (int j=0; j< 16; j++)
		    fprintf(stderr, "%02X ", g_flo2.data[i*16+j]);
    	fprintf(stderr, "\r\n");
	}
}

void dumpTrack()
{
	if( TRACK_FILE == 0)
		if ((TRACK_FILE = fopen("flo2-format.txt","w+")) == NULL)
	    	log_error("Can't open OUTPUT FILE file");

	for (int i=0; i< TRACK_SIZE/16; i++)
	{
		for (int j=0; j< 16; j++)
		    fprintf(TRACK_FILE, "%02X ", g_flo2.trackdata[i*16+j]);
    	fprintf(TRACK_FILE, "\r\n");
	}
   	fprintf(TRACK_FILE, "\r\n");
	if( g_flo2.akt_track == 79 && g_flo2.side == 1 )
		fclose(TRACK_FILE);
}

void readAddress()
{
	g_flo2.data[0] = g_flo2.akt_track;
	g_flo2.data[1] = g_flo2.side;
	g_flo2.data[2] = 0x01;
	g_flo2.data[3] = 0x03;

	WORD_68K crc = 0;
	for( int i=0; i<4; i++) {
	 	crc = crc_add_byte(crc, g_flo2.data[i]);		
	}

	// TODO: Check if endianness is correct
	g_flo2.data[4] = (BYTE_68K) ((crc & 0xFF00) >> 8);
	g_flo2.data[5] = (BYTE_68K) (crc & 0x00FF);
	log_debug("Address field: %2X %2X %2X %2X %2X %2X", g_flo2.data[0], g_flo2.data[1], g_flo2.data[2], g_flo2.data[3], g_flo2.data[4], g_flo2.data[5]);

	g_flo2.offset = 0;
	g_flo2.data_size = 6;
	g_flo2.drq = true;
}

void readSector()
{
	int i;
	FILE* file = NULL;

	if( g_flo2.active_drive >=0 && g_flo2.active_drive < 4)
		file =  g_flo2.disk_files[g_flo2.active_drive];

	if(file == NULL) {                  // verify file is opened
	   	log_error("No File descriptor for file: %1d.", g_flo2.active_drive);
		g_flo2.status = 0x81;
		g_flo2.intrq = true; 
    	return;
	}

	// seek to set sector
	int sector = g_flo2.akt_track * NUM_SECTOR * 2 + NUM_SECTOR * g_flo2.side + (g_flo2.sector - 1 );

	fseek(file, sector * SEC_SIZE, SEEK_SET);
	int pos = ftell(file);
  	if( pos != sector * SEC_SIZE) {
	   	log_error("Read failed Expected: %d, Actual: %2d", sector * SEC_SIZE, pos);
    	return;
	}

	i = fread(&g_flo2.data, 1, SEC_SIZE, file); // read the data

	if(i != SEC_SIZE)
    	g_flo2.status |= STATUS_II_CRC_ERR;

//	dumpSector();
	g_flo2.offset = 0;
	g_flo2.data_size = SEC_SIZE;
	g_flo2.drq = true;
}

void writeSector()
{
	int i;
	FILE* file = NULL;

	if( g_flo2.active_drive >=0 && g_flo2.active_drive < 4)
		file =  g_flo2.disk_files[g_flo2.active_drive];

	if(file == NULL) {                  // verify file is opened
	   	log_error("No File descriptor for file: %1d.", g_flo2.active_drive);
		g_flo2.status = STATUS_II_NOT_FOUND | STATUS_II_DRQ;
		g_flo2.intrq = true; 
    	return;
	}

	// seek to set sector
	int sector = g_flo2.akt_track * NUM_SECTOR * 2 + NUM_SECTOR * g_flo2.side + (g_flo2.sector - 1 );

	fseek(file, sector * SEC_SIZE, SEEK_SET);
	int pos = ftell(file);
  	if( pos != sector * SEC_SIZE) {
	   	log_error("Write failed Expected: %d, Actual: %2d", sector * SEC_SIZE, pos);
    	return;
	}
	//dumpSector();

	int count = SEC_SIZE;
	log_debug("Writing sector %d %d %d %d %d", g_flo2.side, g_flo2.akt_track, g_flo2.track, g_flo2.sector, sector);
	do {
		i = fwrite(&g_flo2.data[0], 1, count, file); 
		if(i == -1) {
			g_flo2.status |= STATUS_II_CRC_ERR;
			return;
		}
		count -= i;
	} while(count > 0);
	fflush(file);

	g_flo2.offset = 0;
	g_flo2.status = 0;
	g_flo2.head_down = true;
}

void writeTrack()
{
	int i;
	FILE* file = NULL;

	if( g_flo2.active_drive >=0 && g_flo2.active_drive < 4)
		file =  g_flo2.disk_files[g_flo2.active_drive];

	if(file == NULL) {                  // verify file is opened
	   	log_error("No File descriptor for file: %1d.", g_flo2.active_drive);
		g_flo2.status = STATUS_II_NOT_FOUND | STATUS_II_DRQ;
		g_flo2.intrq = true; 
    	return;
	}

	// seek to start of track
	int sector = g_flo2.akt_track * NUM_SECTOR * 2 + NUM_SECTOR * g_flo2.side;

	fseek(file, sector * SEC_SIZE, SEEK_SET);
	int pos = ftell(file);
  	if( pos != sector * SEC_SIZE) {
	   	log_error("Write failed Expected: %d, Actual: %2d", sector * SEC_SIZE, pos);
    	return;
	}
	dumpTrack();

	int count = TRACK_SIZE;
	log_debug("Writing track %d %d %d %d %d", g_flo2.side, g_flo2.akt_track, g_flo2.track, g_flo2.sector, sector);
	// do {
	// 	i = fwrite(&g_flo2.data[0], 1, count, file); 
	// 	if(i == -1) {
	// 		g_flo2.status |= STATUS_II_CRC_ERR;
	// 		return;
	// 	}
	// 	count -= i;
	// } while(count > 0);

	g_flo2.offset = 0;
	g_flo2.status = 0;
	g_flo2.head_down = true;
}

BYTE_68K flo2_pC0_in()
{
   	log_debug("Reading FLO2 Status register %02X. Clear Interrupts.", g_flo2.status);
	g_flo2.intrq = false;
	return g_flo2.status;
}

void flo2_pC0_out(BYTE_68K data)
{
   	log_debug("C0 cmd %02X", data);

	g_flo2.status = 0;
	BYTE_68K cmd = data & (BYTE_68K) 0xF0;
	g_flo2.intrq = false;
	switch(cmd)
    {
    case CMD_RESTORE:
    	log_debug("Restore                 : TRACK: %02d %02X", g_flo2.akt_track, data & 0x0F);
		g_flo2.akt_track = 0;
		g_flo2.track = 0;
		g_flo2.status |= STATUS_I_TRACK_0;
		if( data & 0x08) {
			g_flo2.head_down = true;
			g_flo2.status |= STATUS_I_HEAD_LOADED;
		}
		else
			g_flo2.head_down = false;
		if( data & 0x04 )
			runVerify();
		g_flo2.intrq = true;
    	return;
    case CMD_SEEK:
    	log_debug("Seek                    : TRACK: %02d %02X", g_flo2.akt_track, data & 0x0F);
		g_flo2.akt_track = g_flo2.dataword;
		g_flo2.track = g_flo2.dataword;
		if(g_flo2.akt_track == 0)
			g_flo2.status |= STATUS_I_TRACK_0;
		if( data & 0x08) {
			g_flo2.head_down = true;
			g_flo2.status |= STATUS_I_HEAD_LOADED;
		}
		else
			g_flo2.head_down = false;
		if( data & 0x04 )
			runVerify();
		g_flo2.intrq = true;
    	return;
    case CMD_STEP_NOUPD:
    	log_debug("Step no update          : TRACK: %02d %02X", g_flo2.akt_track, data & 0x0F);
		if( g_flo2.step_in )
		{
			if (g_flo2.akt_track < 79) {
				g_flo2.akt_track += 1;
			} else {
				g_flo2.status |= STATUS_I_SEEK_ERR;
			}
		} else {
			if (g_flo2.akt_track > 0) {
				g_flo2.akt_track -= 1;
			} else {
				g_flo2.status |= STATUS_I_SEEK_ERR;
			}
		}
		if(g_flo2.akt_track == 0)
			g_flo2.status |= STATUS_I_TRACK_0;
		if( data & 0x08) {
			g_flo2.head_down = true;
			g_flo2.status |= STATUS_I_HEAD_LOADED;
		}
		else
			g_flo2.head_down = false;
		if( data & 0x04 )
			runVerify();
		g_flo2.intrq = true;
    	return;
    case CMD_STEP_UPD:
    	log_debug("Step update             : TRACK: %02d %02X", g_flo2.akt_track, data & 0x0F);
		if( g_flo2.step_in )
		{
			if (g_flo2.akt_track < 79) {
				g_flo2.akt_track += 1;
				g_flo2.track = g_flo2.akt_track;
			} else {
				g_flo2.status |= STATUS_I_SEEK_ERR;
			}
		} else {
			if (g_flo2.akt_track > 0) {
				g_flo2.akt_track -= 1;
				g_flo2.track = g_flo2.akt_track;
			} else {
				g_flo2.status |= STATUS_I_SEEK_ERR;
			}
		}
		if(g_flo2.akt_track == 0)
			g_flo2.status |= STATUS_I_TRACK_0;
		if( data & 0x08) {
			g_flo2.head_down = true;
			g_flo2.status |= STATUS_I_HEAD_LOADED;
		}
		else
			g_flo2.head_down = false;
		if( data & 0x04 )
			runVerify();
		g_flo2.intrq = true;
		return;
    case CMD_STEP_IN_NOUPD:
    	log_debug("Step in no update       : TRACK: %02d %02X", g_flo2.akt_track, data & 0x0F);
		g_flo2.step_in = true;
		if (g_flo2.akt_track < 79) {
			g_flo2.akt_track += 1;
		} else {
			g_flo2.status |= STATUS_I_SEEK_ERR;
		}
		if( data & 0x08) {
			g_flo2.head_down = true;
			g_flo2.status |= STATUS_I_HEAD_LOADED;
		}
		else
			g_flo2.head_down = false;
		if( data & 0x04 )
			runVerify();
		g_flo2.intrq = true;
    	return;
    case CMD_STEP_IN_UPD:
    	log_debug("Step in update          : TRACK: %02d %02X", g_flo2.akt_track, data & 0x0F);
		g_flo2.step_in = true;
		if (g_flo2.akt_track < 79) {
			g_flo2.akt_track += 1;
			g_flo2.track = g_flo2.akt_track;
		} else {
			g_flo2.akt_track = 0;
			g_flo2.track = g_flo2.akt_track;
//			g_flo2.status |= STATUS_I_SEEK_ERR;
		}
		if( data & 0x08) {
			g_flo2.head_down = true;
			g_flo2.status |= STATUS_I_HEAD_LOADED;
		}
		else
			g_flo2.head_down = false;
		if( data & 0x04 )
			runVerify();
		g_flo2.intrq = true;
    	return;
    case CMD_STEP_OUT_NOUPD:
    	log_debug("Step out no update      : TRACK: %02d %02X", g_flo2.akt_track, data & 0x0F);
		g_flo2.step_in = false;
		if (g_flo2.akt_track > 0) {
			g_flo2.akt_track -= 1;
		} else {
			g_flo2.status |= STATUS_I_SEEK_ERR;
		}
		if(g_flo2.akt_track == 0)
			g_flo2.status |= STATUS_I_TRACK_0;
		if( data & 0x08) {
			g_flo2.head_down = true;
			g_flo2.status |= STATUS_I_HEAD_LOADED;
		}
		else
			g_flo2.head_down = false;
		if( data & 0x04 )
			runVerify();
		g_flo2.intrq = true;
    	return;
    case CMD_STEP_OUT_UPD:
    	log_debug("Step out update         : TRACK: %02d %02X", g_flo2.akt_track, data & 0x0F);
		g_flo2.step_in = false;
		if (g_flo2.akt_track > 0) {
			g_flo2.akt_track -= 1;
			g_flo2.track = g_flo2.akt_track;
		} else {
			g_flo2.status |= STATUS_I_SEEK_ERR;
		}
		if(g_flo2.akt_track == 0)
			g_flo2.status |= STATUS_I_TRACK_0;
		if( data & 0x08) {
			g_flo2.head_down = true;
			g_flo2.status |= STATUS_I_HEAD_LOADED;
		}
		else
			g_flo2.head_down = false;
		if( data & 0x04 )
			runVerify();
		g_flo2.intrq = true;
    	return;
    case CMD_READ_SECT:
    	log_debug("Reading sector          : TRACK: %02d SECTOR:%02d %02X", g_flo2.akt_track, g_flo2.sector, data & 0x0F);
		readSector();
		g_flo2.head_down = true;
    	return;
    case CMD_READ_SECT_MULT:
    	log_debug("Reading multiple sectors: TRACK: %02d SECTOR:%02d %02X", g_flo2.akt_track, g_flo2.sector, data & 0x0F);
//		readSector();
		g_flo2.head_down = true;
    	return;
    case CMD_WRITE_SECT:
    	log_debug("Writnig sector          : TRACK: %02d SECTOR:%02d %02X", g_flo2.akt_track, g_flo2.sector, data & 0x0F);
		g_flo2.writeTrack = false;
		g_flo2.drq = true;
		g_flo2.head_down = true;
    	return;
    case CMD_WRITE_SECT_MULT:
    	log_debug("Writnig multiple sectors: TRACK: %02d SECTOR:%02d %02X", g_flo2.akt_track, g_flo2.sector, data & 0x0F);
		g_flo2.head_down = true;
    	return;
    case CMD_READ_ADDRESS:
    	log_debug("Reading address: TRACK: %02d SECTOR:%02d %02X", g_flo2.akt_track, g_flo2.sector, data & 0x0F);
		readAddress();
		g_flo2.head_down = true;
    	return;
    case CMD_READ_TRACK:
    	log_debug("Reading track: TRACK: %02d SECTOR:%02d %02X", g_flo2.akt_track, g_flo2.sector, data & 0x0F);
		g_flo2.head_down = true;
    	return;
    case CMD_WRITE_TRACK:
    	log_debug("Writnig track: TRACK: %02d SECTOR:%02d %02X", g_flo2.akt_track, g_flo2.sector, data & 0x0F);
    	log_debug("Offset: %04d", g_flo2.offset);
		g_flo2.writeTrack = true;
		g_flo2.head_down = true;
		g_flo2.drq = true;
//		g_flo2.intrq = true;
		usleep(100);
    	return;
    case CMD_FORCE_INT:
    	log_debug("Force Interrupt: %02X", data & 0x0F);
		g_flo2.intrq = true;
    	return;
	default:
    	log_debug("Unknown floppy command %02X", cmd);
		g_flo2.intrq = false;
		return;
	}
}

BYTE_68K flo2_pC1_in()
{
	return g_flo2.track;
}

void flo2_pC1_out(BYTE_68K data)
{
	g_flo2.track = data;
}

BYTE_68K flo2_pC2_in()
{
	return g_flo2.sector;
}

void flo2_pC2_out(BYTE_68K data)
{
	g_flo2.sector = data;
}

BYTE_68K flo2_pC3_in()
{
	BYTE_68K ret = 0;
	if( g_flo2.offset < g_flo2.data_size )
		ret = g_flo2.data[g_flo2.offset++];
	if( g_flo2.offset == g_flo2.data_size )
	{
		if(g_flo2.data_size == 6 )
			fprintf(stderr,"Generating interrupt after data read");
		g_flo2.intrq = true;
		g_flo2.drq = false;
		g_flo2.offset = 0;
	}
	return ret;
}

void flo2_pC3_out(BYTE_68K data)
{
	if( !g_flo2.drq ) {
    	log_debug("Receiving other data %d", data);
		g_flo2.dataword = data;
	} else {
		if(!g_flo2.writeTrack) {
//    	log_debug("Receiving Sector data %d", data);

		if( g_flo2.offset < SEC_SIZE ) {
			g_flo2.data[g_flo2.offset] = data;
			g_flo2.offset++;
		}
		if( g_flo2.offset == SEC_SIZE ) {
			writeSector();
			g_flo2.intrq = true;
			g_flo2.drq = false;
			g_flo2.offset = 0;
		}
		} else {
//			log_debug("Receiving Track data %d", data);

			if( g_flo2.offset < TRACK_SIZE ) {
				g_flo2.trackdata[g_flo2.offset] = data;
				g_flo2.offset++;
			}
			if( g_flo2.offset == TRACK_SIZE ) {
				log_debug("Finished data for Track %d offset, %d", g_flo2.akt_track, g_flo2.offset);
				writeTrack();
				g_flo2.intrq = true;
				g_flo2.drq = false;
				g_flo2.offset = 0;
			}

		}
	}
}

BYTE_68K flo2_pC4_in()
{
	BYTE_68K ret = 0;
	if(g_flo2.head_down)
		ret = ret | 0b00100000; 
	if(g_flo2.intrq)
		ret = ret | 0b01000000; 
	if(g_flo2.drq)
		ret = ret | 0b10000000; 
	return ret;
}

void flo2_pC4_out(BYTE_68K data)
{
	BYTE_68K drive = data & 0x0F;
   	log_debug("Writing special register C4: %X", data);

	switch(drive)
	{
	case 0:
		g_flo2.active_drive = -1;
    	break;
	case 1:
		g_flo2.active_drive = 0;
    	break;
	case 2:
		g_flo2.active_drive = 1;
    	break;
	case 4:
		g_flo2.active_drive = 2;
    	break;
	case 8:
		g_flo2.active_drive = 3;
    	break;
	default:
		g_flo2.active_drive = -1;
    	log_error("Unsupported drive %d %2X", drive, data);
    	break;
	}

	if( (data & 0b10000000 ) != 0 ) {
		g_flo2.side = 1;
	} else { 
		g_flo2.side = 0;
	}
   	log_debug("Selecting drive %d side %d", g_flo2.active_drive, g_flo2.side);
	g_flo2.drive = data;
}

void flo2_reset()
{
	g_flo2.status = 0;
	g_flo2.active_drive = 0;
	g_flo2.data_size = SEC_SIZE;

	g_flo2.head_down = false;
	g_flo2.step_in = false;
	g_flo2.intrq = false;
	g_flo2.drq = false;
	g_flo2.writeTrack = false;

	flo2_close_drives();
	if( g_config.diskA != NULL)
		flo2_open_drive(0, g_config.diskA);
	if( g_config.diskB != NULL)
		flo2_open_drive(1, g_config.diskB);
	if( g_config.diskC != NULL)
		flo2_open_drive(2, g_config.diskC);
	if( g_config.diskD != NULL)
		flo2_open_drive(3, g_config.diskD);

   	log_debug("Resetting FLO2 Controller.");
}

/*
  Open a file for use as a CP/M file system. Must specify the drive number,
  filename, and mode.
 */
void flo2_open_drive(int drive_num, const char *fname)
{
    if (g_flo2.disk_files[drive_num] != NULL)
    {
        fflush(g_flo2.disk_files[drive_num]);
        fclose(g_flo2.disk_files[drive_num]);
    }
    if ((g_flo2.disk_files[drive_num] = fopen(fname, "rb+")) == NULL)
    {
        log_error("Disk image %s doesn't exist! \n", fname);
        g_flo2.disk_size[drive_num] = -1;
        return;
    }
    fseek(g_flo2.disk_files[drive_num], 0, SEEK_END); // make sure at start of file
    g_flo2.disk_size[drive_num] = (int)ftell(g_flo2.disk_files[drive_num]);
    log_info( "Disk %s opend as drive: %c, size %d kByte", 
			  fname, drive_num+'A',
			  g_flo2.disk_size[drive_num]/1024 );
    fseek(g_flo2.disk_files[drive_num], 0, SEEK_SET); // make sure at start of file
}

void flo2_close_drives()
{
    for (int i = 0; i < 4; i++)
    {
        if (g_flo2.disk_files[i] != NULL)
        {
            fseek(g_flo2.disk_files[i], 0, 0);
            fclose(g_flo2.disk_files[i]);
			g_flo2.disk_files[i] = NULL;
        }
    }
}