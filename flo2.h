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

#ifndef HEADER__FLO2
#define HEADER__FLO2
#include "nkc.h"

#define SEC_SIZE 1024
#define TRACK_SIZE 6600
#define	NUM_TRACK 80 
#define NUM_SECTOR 5

#define STATUS_I_NOT_READY   0b10000000
#define STATUS_I_READ_ONLY   0b01000000
#define STATUS_I_HEAD_LOADED 0b00100000
#define STATUS_I_SEEK_ERR    0b00010000
#define STATUS_I_CRC_ERR     0b00001000
#define STATUS_I_TRACK_0     0b00000100
#define STATUS_I_INDEX       0b00000010
#define STATUS_I_BUSY        0b00000001

#define STATUS_II_NOT_READY  0b10000000
#define STATUS_II_READ_ONLY  0b01000000
#define STATUS_II_HEAD_LOAD  0b00100000
#define STATUS_II_NOT_FOUND  0b00010000
#define STATUS_II_CRC_ERR    0b00001000
#define STATUS_II_DATA_LOSS  0b00000100
#define STATUS_II_DRQ        0b00000010
#define STATUS_II_BUSY       0b00000001

#define CMD_RESTORE          0b00000000
#define CMD_SEEK             0b00010000
#define CMD_STEP_NOUPD       0b00100000
#define CMD_STEP_UPD         0b00110000
#define CMD_STEP_IN_NOUPD    0b01000000
#define CMD_STEP_IN_UPD      0b01010000
#define CMD_STEP_OUT_NOUPD   0b01100000
#define CMD_STEP_OUT_UPD     0b01110000
#define CMD_READ_SECT        0b10000000
#define CMD_READ_SECT_MULT   0b10010000
#define CMD_WRITE_SECT       0b10100000
#define CMD_WRITE_SECT_MULT  0b10110000
#define CMD_READ_ADDRESS     0b11000000
#define CMD_READ_TRACK       0b11100000
#define CMD_WRITE_TRACK      0b11110000
#define CMD_FORCE_INT        0b11010000

#define DRIVE_MINI_SD	     0b00110000
#define DRIVE_MINI_DD	     0b00100000
#define DRIVE_MAXI_SD	     0b00010000
#define DRIVE_MAXI_DD	     0b00000000

typedef struct {
    BYTE_68K status;
    BYTE_68K drive;
    BYTE_68K active_drive;
    BYTE_68K akt_track;
    BYTE_68K side;
    BYTE_68K track;
    BYTE_68K sector;
    BYTE_68K dataword;
    BYTE_68K data[SEC_SIZE];
    BYTE_68K trackdata[TRACK_SIZE];
    int offset;
    int data_size;

    bool head_down;
    bool step_in;
    bool intrq;
    bool drq;
    bool writeTrack;

    FILE *disk_files[4];
    int disk_size[4];
    FILE* trackFile;

} flo2;


#ifdef __cplusplus
extern "C"
{
#endif

    BYTE_68K flo2_pC0_in(); /* Status register */
    void flo2_pC0_out(BYTE_68K data);
    BYTE_68K flo2_pC1_in(); /* Track register */
    void flo2_pC1_out(BYTE_68K data);
    BYTE_68K flo2_pC2_in(); /* Sector register */
    void flo2_pC2_out(BYTE_68K data);
    BYTE_68K flo2_pC3_in(); /* Data register */
    void flo2_pC3_out(BYTE_68K data);
    BYTE_68K flo2_pC4_in(); /* Drive type special register */
    void flo2_pC4_out(BYTE_68K data);
    void flo2_reset();
    void flo2_close_drives();
    void flo2_open_drive(int drive_num, const char *fname);
    void flo2_close_drives();

#ifdef __cplusplus
}
#endif

#endif /* HEADER__FLO2 */