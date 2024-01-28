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
#include <yaml.h>
#include "config.h"
#include "sound.h"
#include "ioe.h"
#include "cas.h"
#include "promer.h"
#include "centronics.h"
#include "flo2.h"

config g_config;
long g_romAddr = -1;

void logEmitterError(yaml_emitter_t * emitter, yaml_event_t * event)
{
    log_error("Failed to emit event %d: %s\n", event->type, emitter->problem);
}

void emitConfigEntry(yaml_emitter_t * emitter, const char* name, const char* value)
{
    yaml_event_t event;

    yaml_mapping_start_event_initialize(&event, NULL, (yaml_char_t *)YAML_MAP_TAG,
            1, YAML_ANY_MAPPING_STYLE);
    if (!yaml_emitter_emit(emitter, &event))
        logEmitterError(emitter, &event);

    yaml_scalar_event_initialize(&event, NULL, (yaml_char_t *)YAML_STR_TAG,
        (yaml_char_t *)name, strlen(name), 1, 0, YAML_PLAIN_SCALAR_STYLE);
    if (!yaml_emitter_emit(emitter, &event))
        logEmitterError(emitter, &event);

    char * val = (char *) value;
    if( val == NULL)
        val = "";

    yaml_scalar_event_initialize(&event, NULL, (yaml_char_t *)YAML_STR_TAG,
        (yaml_char_t *)val, strlen(val), 1, 0, YAML_PLAIN_SCALAR_STYLE);
    if (!yaml_emitter_emit(emitter, &event))
        logEmitterError(emitter, &event);

    yaml_mapping_end_event_initialize(&event);
    if (!yaml_emitter_emit(emitter, &event))
        logEmitterError(emitter, &event);
}

int getRomIndex(long addr)
{
    int index = -1;
    switch (addr)
    {
    case 0xA0000:
        index = 0;
        break;
    case 0xA2000:
        index = 1;
        break;
    case 0xA4000:
        index = 2;
        break;
    case 0xA6000:
        index = 3;
        break;
    case 0xA8000:
        index = 4;
        break;
    case 0xAA000:
        index = 5;
        break;
    case 0xAC000:
        index = 6;
        break;
    case 0xAE000:
        index = 7;
        break;
    case 0xB0000:
        index = 8;
        break;
    case 0xB2000:
        index = 9;
        break;
    case 0xB4000:
        index = 10;
        break;
    case 0xB6000:
        index = 11;
        break;
    case 0xB8000:
        index = 12;
        break;
    case 0xBA000:
        index = 12;
        break;
    case 0xBC000:
        index = 14;
        break;
    case 0xBE000:
        index = 15;
        break;
    case 0xC0000:
        index = 16;
        break;
    case 0xC2000:
        index = 17;
        break;
    case 0xC4000:
        index = 18;
        break;
    case 0xC6000:
        index = 19;
        break;
    case 0xC8000:
        index = 20;
        break;
    case 0xCA000:
        index = 21;
        break;
    case 0xCC000:
        index = 22;
        break;
    case 0xCE000:
        index = 23;
        break;
    case 0xD0000:
        index = 24;
        break;
    case 0xD2000:
        index = 25;
        break;
    case 0xD4000:
        index = 26;
        break;
    case 0xD6000:
        index = 27;
        break;
    case 0xD8000:
        index = 28;
        break;
    case 0xDA000:
        index = 29;
        break;
    case 0xDC000:
        index = 30;
        break;
    case 0xDE000:
        index = 31;
        break;
    case 0xE0000:
        index = 32;
        break;
    case 0xE2000:
        index = 33;
        break;
    case 0xE4000:
        index = 34;
        break;
    case 0xE6000:
        index = 35;
        break;
    }
    return index;
}

long getRomAddress(const char *key)
{
    long addr = -1L;
    char hexAddress[6];
    char *endptr;

    if (strlen(key) == 13)
    {
        strncpy(hexAddress, key + 8, 5);
        hexAddress[5] = '\0';
        addr = strtol(hexAddress, &endptr, 16);
        if (*endptr != '\0') /* Some error occured converting the string */
            addr = -1L;
    }
    return addr;
}

int getKeyType(const char *key)
{
    if (strcmp(key, "CPUSpeed") == 0)
        return CPU_SPEED_MHZ;
    if (strcmp(key, "NumWaitStates") == 0)
        return NUM_WAIT_STATES;
    if (strcmp(key, "GDP64XMag") == 0)
        return GDP64_X_MAG;
    if (strcmp(key, "GDP64YMag") == 0)
        return GDP64_Y_MAG;
    if (strcmp(key, "Col256XMag") == 0)
        return COL256_X_MAG;
    if (strcmp(key, "Col256YMag") == 0)
        return COL256_Y_MAG;
    if (strcmp(key, "Col256RAM") == 0)
        return COL256_RAM;
    if (strcmp(key, "KeyDILSwitches") == 0)
        return KEY_DIL_SWITCHES;
    if (strcmp(key, "SoundDriver") == 0)
        return SOUND_DRIVER;
    if (strcmp(key, "CasFile") == 0)
        return CAS_FILE;
    if (strcmp(key, "ListFile") == 0)
        return LST_FILE;
    if (strcmp(key, "PromFile") == 0)
        return PROM_FILE;
    if (strcmp(key, "JoystickA") == 0)
        return JOYSTICK_A;
    if (strcmp(key, "JoystickB") == 0)
        return JOYSTICK_B;
    if (strcmp(key, "BankBootRom") == 0)
        return BANKBOOT_ROM;
    if (strncmp(key, "RomPath_", 8) == 0)
    {
        g_romAddr = getRomAddress(key);
        if (g_romAddr == -1L)
            return CONFIG_UNKNOWN;
        return ROM_PATH;
    }
    if (strncmp(key, "RomSize_", 8) == 0)
    {
        g_romAddr = getRomAddress(key);
        if (g_romAddr == -1L)
            return CONFIG_UNKNOWN;
        return ROM_SIZE;
    }
    if (strcmp(key, "DriveA") == 0)
        return DISK_A;
    if (strcmp(key, "DriveB") == 0)
        return DISK_B;
    if (strcmp(key, "DriveC") == 0)
        return DISK_C;
    if (strcmp(key, "DriveD") == 0)
        return DISK_D;

    return CONFIG_UNKNOWN;
}

void readConfig(const char *config_file)
{
    FILE *fh = fopen(config_file, "r");
    yaml_parser_t parser;
    yaml_token_t token; /* new variable */
    int type;
    int romIndex;
    /*  state = 0 = expect key
     *  state = 1 = expect value
     */
    int state = 0;

    g_config.simSpeed = 0;          // Default to run in Turbo mode
    g_config.cpuSpeed = 8;          // Simulated CPU Speed in MHz
    g_config.setINT = 0;            // Default to not to connect the vertical blank signal with the INT line
    g_config.setNMI = 1;            // Default to connect the INT and NMI lines together to generate a level 7 interrupt
    g_config.numWaitStates = 3;     // Default to 3 wait states

    /* Initialize parser */
    if (!yaml_parser_initialize(&parser))
    {
        log_error("Failed to initialize configuration file parser!");
        return;
    }
    if (fh == NULL)
    {
        log_error("Failed to open config file %s!",config_file);
        return;
    }

    /* Set input file */
    yaml_parser_set_input_file(&parser, fh);

    /* BEGIN new code */
    do
    {
        char *tk;

        yaml_parser_scan(&parser, &token);

        switch (token.type)
        {
        case YAML_KEY_TOKEN:
            state = 0;
            break;
        case YAML_VALUE_TOKEN:
            state = 1;
            break;
        case YAML_SCALAR_TOKEN:
            tk = (char *)token.data.scalar.value;
            if (state == 0)
            {
                type = getKeyType(tk);
            }
            else
            {
                switch (type)
                {
                case CPU_SPEED_MHZ:
                    g_config.cpuSpeed = strtol(tk, NULL, 0);
                    break;
                case NUM_WAIT_STATES:
                    g_config.numWaitStates = strtol(tk, NULL, 0);
                    break;
                case GDP64_X_MAG:
                    g_config.gdp64XMag = strtol(tk, NULL, 0);
                    break;
                case GDP64_Y_MAG:
                    g_config.gdp64YMag = strtol(tk, NULL, 0);
                    break;
                case COL256_X_MAG:
                    g_config.col256XMag = strtol(tk, NULL, 0);
                    break;
                case COL256_RAM:
                    g_config.col256RAMAddr = strtol(tk, NULL, 0);
                    break;
                case COL256_Y_MAG:
                    g_config.col256YMag = strtol(tk, NULL, 0);
                    break;
                case KEY_DIL_SWITCHES:
                    g_config.keyDILSwitches = strtol(tk, NULL, 0);
                    break;
                case SOUND_DRIVER:
                    g_config.soundDriver = strdup(tk);
                    break;
                case CAS_FILE:
                    g_config.casFile = strdup(tk);
                    break;
                case LST_FILE:
                    g_config.listFile = strdup(tk);
                    break;
                case PROM_FILE:
                    g_config.promFile = strdup(tk);
                    break;
                case JOYSTICK_A:
                    g_config.joystickA = strdup(tk);
                    break;
                case JOYSTICK_B:
                    g_config.joystickB = strdup(tk);
                    break;
                case BANKBOOT_ROM:
                    g_config.bankBootRom = strdup(tk);
                    break;
                case ROM_PATH:
                    romIndex = getRomIndex(g_romAddr);
                    if (romIndex >= 0 && romIndex < MAX_ROMS)
                    {
                        g_config.roms[romIndex].path = strdup(tk);
                        g_config.roms[romIndex].start = g_romAddr;
                    }
                    break;
                case ROM_SIZE:
                    romIndex = getRomIndex(g_romAddr);
                    if (romIndex >= 0 && romIndex < MAX_ROMS)
                        g_config.roms[romIndex].size = strtol(tk, NULL, 0);
                    break;
                case DISK_A:
                    g_config.diskA = strdup(tk);
                    break;
                case DISK_B:
                    g_config.diskB = strdup(tk);
                    break;
                case DISK_C:
                    g_config.diskC = strdup(tk);
                    break;
                case DISK_D:
                    g_config.diskD = strdup(tk);
                    break;
                }
            }
            break;
        default:
            break;	
        }
        if (token.type != YAML_STREAM_END_TOKEN)
            yaml_token_delete(&token);
    } while (token.type != YAML_STREAM_END_TOKEN);
    yaml_token_delete(&token);
    /* END new code */

    /* Cleanup */
    yaml_parser_delete(&parser);
    fclose(fh);
    return;
}

void saveConfig(const char *config_file)
{
    char * path;
    char token[20];
    char value[20];

    FILE *fh = fopen(config_file, "w");
    if(fh == NULL)
        log_error("Could not save new config to file %s", config_file);

    
    yaml_emitter_t emitter;
    yaml_event_t event;

    yaml_emitter_initialize(&emitter);
    yaml_emitter_set_output_file(&emitter, fh);

    yaml_stream_start_event_initialize(&event, YAML_UTF8_ENCODING);
    if (!yaml_emitter_emit(&emitter, &event))
        logEmitterError(&emitter, &event);

    yaml_document_start_event_initialize(&event, NULL, NULL, NULL, 0);
    if (!yaml_emitter_emit(&emitter, &event))
        logEmitterError(&emitter, &event);

    yaml_sequence_start_event_initialize(&event, NULL, (yaml_char_t *)YAML_SEQ_TAG,
       1, YAML_ANY_SEQUENCE_STYLE);
    if (!yaml_emitter_emit(&emitter, &event))
        logEmitterError(&emitter, &event);

    // Write Config elements
    sprintf(value,"%u", g_config.cpuSpeed);
    emitConfigEntry(&emitter, "CPUSpeed",value);
    sprintf(value,"%u", g_config.numWaitStates);
    emitConfigEntry(&emitter, "NumWaitStates",value);
    sprintf(value,"%u", g_config.gdp64XMag);
    emitConfigEntry(&emitter, "GDP64XMag",value);
    sprintf(value,"%u", g_config.gdp64YMag);
    emitConfigEntry(&emitter, "GDP64YMag",value);
    sprintf(value,"%u", g_config.col256XMag);
    emitConfigEntry(&emitter, "Col256XMag",value);
    sprintf(value,"%u", g_config.col256YMag);
    emitConfigEntry(&emitter, "Col256YMag",value);
    sprintf(value,"0x%08X", g_config.col256RAMAddr);
    emitConfigEntry(&emitter, "Col256RAM",value);
    sprintf(value,"0x%02X", g_config.keyDILSwitches);
    emitConfigEntry(&emitter, "KeyDILSwitches",value);

    emitConfigEntry(&emitter, "SoundDriver", g_config.soundDriver);
    emitConfigEntry(&emitter, "CasFile", g_config.casFile);
    emitConfigEntry(&emitter, "ListFile", g_config.listFile);
    emitConfigEntry(&emitter, "PromFile", g_config.promFile);
    emitConfigEntry(&emitter, "JoystickA", g_config.joystickA);
    emitConfigEntry(&emitter, "JoystickB", g_config.joystickB);
    emitConfigEntry(&emitter, "BankBootRom", g_config.bankBootRom);

    int i = 0;
    for (int address = 0xA0000; address < 0xE8000; address += 0x2000) {

        // Write RomPath
        sprintf(token,"RomPath_%05X", address);
        emitConfigEntry(&emitter, token, g_config.roms[i].path);
        // Write RomSize
        sprintf(token,"RomSize_%05X", address);
        sprintf(value,"%lu", g_config.roms[i].size);
        emitConfigEntry(&emitter, token,value);
        i++;
    }

    // Write Floppy Drives
    emitConfigEntry(&emitter, "DriveA", g_config.diskA);
    emitConfigEntry(&emitter, "DriveB", g_config.diskB);
    emitConfigEntry(&emitter, "DriveC", g_config.diskC);
    emitConfigEntry(&emitter, "DriveD", g_config.diskD);

    // End document
    yaml_sequence_end_event_initialize(&event);
    if (!yaml_emitter_emit(&emitter, &event))
        logEmitterError(&emitter, &event);

    yaml_document_end_event_initialize(&event, 0);
    if (!yaml_emitter_emit(&emitter, &event))
        logEmitterError(&emitter, &event);

    yaml_stream_end_event_initialize(&event);
    if (!yaml_emitter_emit(&emitter, &event))
        logEmitterError(&emitter, &event);

    yaml_emitter_delete(&emitter);
    fclose(fh);
    return;
}

void setAudioDevice(const char *device_name)
{
    free(g_config.soundDriver);
    g_config.soundDriver = strdup(device_name);
    sound_reset(g_config.soundDriver);
}

void setJoystickA(const char *device_name)
{
    free(g_config.joystickA);
    g_config.joystickA = strdup(device_name);
    ioe_reset(g_config.joystickA, g_config.joystickB);
}

void setJoystickB(const char *device_name)
{
    free(g_config.joystickB);
    g_config.joystickB = strdup(device_name);
    ioe_reset(g_config.joystickA, g_config.joystickB);
}

void setCasFileName(const char *file_name)
{
    free(g_config.casFile);
    g_config.casFile = strdup(file_name);
    cas_setFile(g_config.casFile);
}

void setPromFileName(const char *file_name)
{
    free(g_config.promFile);
    g_config.promFile = strdup(file_name);
    promer_setFile(g_config.promFile);
}

void setListFileName(const char *file_name)
{
    free(g_config.listFile);
    g_config.listFile = strdup(file_name);
    cent_setFile(g_config.listFile);
}

void setFloppyA(const char *floppy_image)
{
    free(g_config.diskA);
    g_config.diskA = strdup(floppy_image);
    flo2_open_drive(0, g_config.diskA);
}

void setFloppyB(const char *floppy_image)
{
    free(g_config.diskB);
    g_config.diskB = strdup(floppy_image);
    flo2_open_drive(1, g_config.diskB);
}

void setFloppyC(const char *floppy_image)
{
    free(g_config.diskB);
    g_config.diskB = strdup(floppy_image);
    flo2_open_drive(2, g_config.diskB);
}

void setFloppyD(const char *floppy_image)
{
    free(g_config.diskB);
    g_config.diskB = strdup(floppy_image);
    flo2_open_drive(3, g_config.diskB);
}
