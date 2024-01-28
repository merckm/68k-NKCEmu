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

#ifndef HEADER__CONFIG
#define HEADER__CONFIG

#define CPU_SPEED_MHZ 1
#define NUM_WAIT_STATES 2
#define GDP64_X_MAG 3
#define GDP64_Y_MAG 4
#define COL256_X_MAG 5
#define COL256_Y_MAG 6
#define COL256_RAM 7
#define KEY_DIL_SWITCHES 8

#define SOUND_DRIVER 11
#define CAS_FILE 12
#define LST_FILE 13
#define PROM_FILE 14
#define JOYSTICK_A 15
#define JOYSTICK_B 16
#define BANKBOOT_ROM 17
#define ROM_PATH 18
#define ROM_SIZE 19
#define DISK_A 20
#define DISK_B 21
#define DISK_C 22
#define DISK_D 23
#define CONFIG_UNKNOWN 1000
#define MAX_ROMS 36

typedef struct {
    char * path;
    unsigned long start;
    unsigned long size;
} rom;


typedef struct {
	int simSpeed;
	int cpuSpeed;
	int numWaitStates;
	int setINT;
	int setNMI;
	int gdp64XMag;
	int gdp64YMag;
	int col256XMag;
	int col256YMag;
	int col256RAMAddr;
	int keyDILSwitches;
	char * soundDriver;
	char * casFile;
	char * listFile;
	char * promFile;
	char * joystickA;
	char * joystickB;
	char * bankBootRom;
	rom roms[MAX_ROMS];
	char * diskA;
	char * diskB;
	char * diskC;
	char * diskD;
} config;

#ifdef __cplusplus
extern "C"
{
#endif

    void readConfig(const char * config_file);
    void saveConfig(const char * config_file);
    void setAudioDevice(const char * device_name);
    void setJoystickA(const char * device_name);
    void setJoystickB(const char * device_name);
    void setCasFileName(const char * file_name);
    void setPromFileName(const char * file_name);
    void setListFileName(const char * file_name);
    void setFloppyA(const char * floppy_image);
    void setFloppyB(const char * floppy_image);
    void setFloppyC(const char * floppy_image);
    void setFloppyD(const char * floppy_image);

#ifdef __cplusplus
}
#endif

#endif /* HEADER__CONFIG */