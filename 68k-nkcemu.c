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

/*
 * MC68000 simulation of the NDR Klein Computer
 *
 * Uses the example that came with the Musashi simulator as a skeleton to
 * build on.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <yaml.h>

/* OS-dependant code to get a character from the user.
 */
#include <sys/types.h>
#include <sys/time.h>

#include "nkc.h"
#include "68k-nkcemu.h"
#include "nkcgui.h"
#include "config.h"
#include "m68k.h"
#include "bankboot.h"
#include "key.h"
#include "gdp64.h"
#include "col256.h"
#include "bankboot.h"
#include "flo2.h"
#include "ioe.h"
#include "cas.h"
#include "centronics.h"
#include "promer.h"
#include "sound.h"
#include "uhr.h"

/* Read/write macros */
#define READ_BYTE(BASE, ADDR) (BASE)[ADDR]
#define READ_WORD(BASE, ADDR) (((BASE)[ADDR] << 8) | \
                               (BASE)[(ADDR) + 1])
#define READ_LONG(BASE, ADDR) (((BASE)[ADDR] << 24) |       \
                               ((BASE)[(ADDR) + 1] << 16) | \
                               ((BASE)[(ADDR) + 2] << 8) |  \
                               (BASE)[(ADDR) + 3])

#define WRITE_BYTE(BASE, ADDR, VAL) (BASE)[ADDR] = (VAL) & 0xff
#define WRITE_WORD(BASE, ADDR, VAL)     \
    (BASE)[ADDR] = ((VAL) >> 8) & 0xff; \
    (BASE)[(ADDR) + 1] = (VAL) & 0xff
#define WRITE_LONG(BASE, ADDR, VAL)            \
    (BASE)[ADDR] = ((VAL) >> 24) & 0xff;       \
    (BASE)[(ADDR) + 1] = ((VAL) >> 16) & 0xff; \
    (BASE)[(ADDR) + 2] = ((VAL) >> 8) & 0xff;  \
    (BASE)[(ADDR) + 3] = (VAL) & 0xff

extern config g_config;
extern bankboot g_bb;
extern flo2 g_flo2;
extern cas g_cas;
extern col256 g_col;
extern gdp64 g_gdp;
extern cent g_cent;
extern promer g_promer;

int g_start_gp_ram = 0x0E0000;

int g_gdpWindowId;
int g_colWindowId;
int g_guiWindowId;

int g_extraSlice = 0;

/* Prototypes */
// void exit_error(char *fmt, ...);

// unsigned int cpu_read_byte(unsigned int address);
// unsigned int cpu_read_word(unsigned int address);
// unsigned int cpu_read_long(unsigned int address);
// void cpu_write_byte(unsigned int address, unsigned int value);
// void cpu_write_word(unsigned int address, unsigned int value);
// void cpu_write_long(unsigned int address, unsigned int value);
// void cpu_pulse_reset(void);
// void cpu_set_fc(unsigned int fc);
// int cpu_irq_ack(int level);

// void nmi_device_reset(void);
// void nmi_device_update(void);
// int nmi_device_ack(void);

void int_controller_set(unsigned int value);
void int_controller_clear(unsigned int value);

// void nkc_reset();

/* Data */
unsigned int g_quit = 0; /* 1 if we want to quit */
unsigned int g_nmi = 0;  /* 1 if nmi pending */

int g_trace = 0;
bool g_traceFunc = false;

unsigned int g_int_controller_pending = 0;     /* list of pending interrupts */
unsigned int g_int_controller_highest_int = 0; /* Highest pending interrupt */

unsigned char g_rom[MAX_BBROM + 1]; /* ROM */
unsigned char g_ram[MAX_RAM + 1];   /* RAM */

unsigned int g_fc; /* Current function code from CPU */

struct timeval oldtime;
struct timeval oldtime2;
struct timeval akttime;

/* -------------------------------------------------------------------- */
/* Helper fuunctions                                                    */
/* -------------------------------------------------------------------- */
void memdump(int start, int end)
{
    int i = 0;
    while (start < end)
    {
        if ((i++ & 0x0f) == 0)
            fprintf( stdout,"\r\n%08x:", start);
        fprintf( stdout,"%02x ", g_ram[start++]);
    }
}

void termination_handler(int signum)
{
    int i;

    flo2_close_drives();
    saveConfig("./config.yaml");

    log_info( "Final PC=%08x", m68k_get_reg(NULL, M68K_REG_PC));

    exit(0);
}

/*
  Print some information on the instruction and state.
 */
void trace()
{
    static char buff[100];
    static char buff2[100];
    static unsigned int pc, instr_size;

    g_traceFunc = true;
    pc = m68k_get_reg(NULL, M68K_REG_PC);

    char buf[256];

    m68k_disassemble(buf, pc, M68K_CPU_TYPE_68000);
    if (pc < 0xFFFFFF)
    {
        fprintf( stdout,"%06x:%-22s   A0:%08x A1:%08x A2:%08x A3:%08x A4:%08x A5:%08x A6:%08x A7:%08x\r\n", pc, buf,
                m68k_get_reg(NULL, M68K_REG_A0),
                m68k_get_reg(NULL, M68K_REG_A1),
                m68k_get_reg(NULL, M68K_REG_A2),
                m68k_get_reg(NULL, M68K_REG_A3),
                m68k_get_reg(NULL, M68K_REG_A4),
                m68k_get_reg(NULL, M68K_REG_A5),
                m68k_get_reg(NULL, M68K_REG_A6),
                m68k_get_reg(NULL, M68K_REG_A7));
        fprintf( stdout,"       SR:%08x              D0:%08x D1:%08x D2:%08x D3:%08x D4:%08x D5:%08x D6:%08x D7:%08x\r\n",
                m68k_get_reg(NULL, M68K_REG_SR),
                m68k_get_reg(NULL, M68K_REG_D0),
                m68k_get_reg(NULL, M68K_REG_D1),
                m68k_get_reg(NULL, M68K_REG_D2),
                m68k_get_reg(NULL, M68K_REG_D3),
                m68k_get_reg(NULL, M68K_REG_D4),
                m68k_get_reg(NULL, M68K_REG_D5),
                m68k_get_reg(NULL, M68K_REG_D6),
                m68k_get_reg(NULL, M68K_REG_D7));
        fflush(stdout);

    }
    g_traceFunc = false;

    if (pc > 0xFFFF00)
        termination_handler(0);
}

void nkc_reset(void)
{
    flo2_close_drives();
    saveConfig("./config.yaml");

    m68k_pulse_reset();
    cpu_pulse_reset();
}

/* Exit with an error message.  Use printf syntax. */
void exit_error(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");

    exit(EXIT_FAILURE);
}

void init_windows(bool windowed)
{
    int i;
    /* Initialize Graphics Window */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) == -1)
    {
        log_error("Can't init SDL:  %s\n", SDL_GetError());
        exit_error("Can't initialize SDL. Error: %s",SDL_GetError());
    }
    atexit(SDL_Quit);

    SDL_ShowCursor(SDL_TRUE);

    g_gdpWindowId = gdp64_init();
    g_guiWindowId = gui_init();
    g_colWindowId = col_init();
    SDL_RaiseWindow(g_gdp.window);
}

void handle_event()
{
    SDL_Event event;
    /* first read Key if available */
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            SDL_Quit();
            termination_handler(0);
        }

        if (event.type == SDL_WINDOWEVENT)
        {
            if (event.window.event == SDL_WINDOWEVENT_CLOSE)
            {
                SDL_Quit();
                termination_handler(0);
            }
        }
        if (event.window.windowID == g_gdpWindowId)
        {
            gdp64_event(&event);

            if (event.type == SDL_KEYDOWN)
            {
                /* first evaluate simulation keys */
                if (event.key.keysym.sym == SDLK_F2 && g_cas.cas_file != NULL)
                {
                    fseek(g_cas.cas_file, 0, SEEK_SET);
                    g_cas.pos = 0;
                }
                if (event.key.keysym.sym == SDLK_F3)
                {
                    nkc_reset();
                }
                if (event.key.keysym.sym == SDLK_F4)
                {
                    toggle_trace();
                }
            }
            if (g_gdp.isGuiScreen)
                gui_event(&event);
            else
                key_event(&event);
            if (event.type == SDL_JOYAXISMOTION)
                log_debug("Joystick-Event");
            ioe_event(&event);
        }

        if (event.window.windowID == g_guiWindowId)
        {
            if (!g_gdp.isGuiScreen && event.type == SDL_KEYDOWN)
                key_event(&event);
            else
                gui_event(&event);
        }
        if( (event.type == SDL_JOYAXISMOTION) ||
            (event.type == SDL_JOYBUTTONDOWN) ||
            (event.type == SDL_JOYBUTTONUP) )
        {
            ioe_event(&event);
        }
    }
}

/* RAM is hard coded here from 0-512kB, and 32kB after the system EPROMs.                 */
/* For COL256 16kB memory is mapped to RAM from 0xCC000 to 0xD0000 or 0xEC000 to 0xF0000. */
bool isRam(unsigned int address)
{
    if (g_bb.bb_enabled)
        if (address < 0x8000)
            return false;
    if (address >= 0x0 && address < 0x80000) // Ram für CP/M (512 K)
        return true;
    if ((address >= g_config.col256RAMAddr) && (address < g_config.col256RAMAddr + 0x3FFF) && g_col.col_active) // Ram for Col256
        return true;
    if ((address >= g_start_gp_ram) && (address < g_start_gp_ram + 0x8000)) // Ram für das Grundprogramm
        return true;
    return false;
}

/* Read data from RAM */
unsigned int cpu_read_byte(unsigned int address)
{
    if( g_traceFunc == false )
        g_extraSlice += g_config.numWaitStates;

    // Read Memory from COL 256
    if ((address >= g_config.col256RAMAddr) && (address <= g_config.col256RAMAddr + 0x3FFF) && g_col.col_active)
        return col_getPixel(address);

    if (address > 0xffff00)
    {
        switch (address)
        {
        case IOE_PORT_A:
            return ioe_p30_in();
        case IOE_PORT_B:
            return ioe_p31_in();
        case SOUND_ADR:
        case SOUND_JADOS_ADR:
            return sound_p40_in();
        case SOUND_DATA:
        case SOUND_JADOS_DATA:
            return sound_p41_in();
        case CENT_DAT:
            return cent_p48_in();
            return 0;
        case CENT_STB:
            return cent_p49_in();
            return 0;
        case KEY_DATA:
            return key_p68_in();
        case KEY_DIP:
            return key_p69_in();
        case GDP_PAGE:
            return gdp64_p60_in();
        case GDP_CMD:
            return gdp64_p70_in();
        case GDP_CMD + 1:
            return gdp64_p71_in();
        case GDP_CMD + 2:
            return gdp64_p72_in();
        case GDP_CMD + 3:
            return gdp64_p73_in();
        case GDP_CMD + 4:
            return 0;
        case GDP_CMD + 5:
            return gdp64_p75_in();
        case GDP_CMD + 6:
            return 0;
        case GDP_CMD + 7:
            return gdp64_p77_in();
        case GDP_CMD + 8:
            return gdp64_p78_in();
        case GDP_CMD + 9:
            return gdp64_p79_in();
        case GDP_CMD + 10:
            return gdp64_p7A_in();
        case GDP_CMD + 11:
            return gdp64_p7B_in();
        case GDP_CMD + 12:
            return 0;
        case GDP_CMD + 13:
            return 0;
        case GDP_CMD + 14:
            return 0;
        case GDP_CMD + 15:
            return 0;
        case PROM_DAT:
            return promer_p80_in();
        case PROM_A1:
            return promer_p81_in();
        case PROM_A2:
            return promer_p82_in();
        case FLO2_CMD:
            return flo2_pC0_in();
        case FLO2_TRACK:
            return flo2_pC1_in();
        case FLO2_SECT:
            return flo2_pC2_in();
        case FLO2_DATA:
            return flo2_pC3_in();
        case FLO2_ADDI:
            return flo2_pC4_in();
        case BANKBOOT:
            log_debug("Reading BANKBOOT register %#010x", address);
            return bank_pC8_in();
        case CAS_CMD:
            return cas_pCA_in();
        case CAS_DATA:
            return cas_pCB_in();
        case COL_ADDR:
        case COL_JADOS_ADDR:
            return col_pCC_in();
        case COL_DATA:
        case COL_JADOS_DATA:
            return col_pCD_in();
        case COL_PAGE:
        case COL_JADOS_PAGE:
            return col_pCE_in();
        case UHR_DATA:
            return uhr_pFE_in();
        case UNKNOWN:
            return 0;
        default:
            log_debug("I/O byte read from UNKNOWN address %#010x", address);
            break;
        }
    }

    if (g_bb.bb_enabled && address <= 0x2000)
        return READ_BYTE(g_rom, address);
    else
    {
        if (address <= MAX_RAM)     // HINT: g_ram contains RAM and standard ROMs, g_rom only Bankboot rom
            return READ_BYTE(g_ram, address);
        else
            return 0xFF;

    }
}

unsigned int cpu_read_word(unsigned int address)
{
    if( g_traceFunc == false )
        g_extraSlice += (4 + (2 * g_config.numWaitStates));

    if (g_bb.bb_enabled && address <= 0x2000)
        return READ_WORD(g_rom, address);

    if (address > 0xffff00)
    {
        switch (address)
        {
        case GDP_CMD + 8:
            return gdp64_p78_in_word();
        case GDP_CMD + 10:
            return gdp64_p7A_in_word();
        default:
            log_debug("I/O word read from UNKNOWN address %#010x", address);
            break;
        }
    }

    // Read Memory from COL 256
    if ((address >= g_config.col256RAMAddr) && (address <= g_config.col256RAMAddr + 0x3FFF) && g_col.col_active)
        return col_getWord(address);

    if(address <= MAX_RAM)
        return READ_WORD(g_ram, address);
    else
        return 0xFFFF;
}

unsigned int cpu_read_long(unsigned int address)
{
    if( g_traceFunc == false )
        g_extraSlice += (8 + (4 * g_config.numWaitStates));

    if (g_bb.bb_enabled && address <= 0x2000)
        return READ_LONG(g_rom, address);

    if (address > 0xffff00)
    {
        switch (address)
        {
        default:
            log_debug("I/O long read from UNKNOWN address %#010x", address);
            break;
        }
    }

    // Read Memory from COL 256
    if ((address >= g_config.col256RAMAddr) && (address <= g_config.col256RAMAddr + 0x3FFF) && g_col.col_active)
        return col_getLong(address);

    // if(address >= 0x000064 && address <= 0x00007C) {
    //     log_debug("Reading 68000 interrupt vector %08X,  %08X", address, READ_LONG(g_ram, address));
    // }

    if(address <= MAX_RAM)
        return READ_LONG(g_ram, address);
    else
        return 0xFFFFFFFF;
}

/* Write data to RAM or a device */
void cpu_write_byte(unsigned int address, unsigned int value)
{
    if( g_traceFunc == false )
        g_extraSlice += g_config.numWaitStates;

    // Set Pixel in COL256 memory
    if ((address >= g_config.col256RAMAddr) && (address <= g_config.col256RAMAddr + 0x3FFF) && g_col.col_active)
        col_setPixel(address, value & 0xff);

    if (address > 0xffff00)
    {
        switch (address)
        {
        case IOE_PORT_A:
            ioe_p30_out(value & 0xff);
            return;
        case IOE_PORT_B:
            ioe_p31_out(value & 0xff);
            return;
        case SOUND_ADR:
        case SOUND_JADOS_ADR:
            sound_p40_out(value & 0xff);
            return;
        case SOUND_DATA:
        case SOUND_JADOS_DATA:
            sound_p41_out(value & 0xff);
            return;
        case CENT_DAT:
            cent_p48_out(value & 0xff);
            return;
        case CENT_STB:
            cent_p49_out(value & 0xff);
            return;
        case KEY_DATA:
            key_p68_out(value & 0xff);
            return;
        case COLOR_A0:
        case COLOR_A1:
            return;
        case KEY_DIP:
            key_p69_out(value & 0xff);
            return;
        case GDP_PAGE:
            gdp64_p60_out(value & 0xff);
            return;
        case GDP_CMD:               // Status register
            gdp64_p70_out(value & 0xff);
            return;
        case GDP_CMD + 1:           // CTRL1 register
            gdp64_p71_out(value & 0xff);
            return;
        case GDP_CMD + 2:           // CTRL2 register
            gdp64_p72_out(value & 0xff);
            return;
        case GDP_CMD + 3:           // CSIZE register
            gdp64_p73_out(value & 0xff);
            return;
        case GDP_CMD + 4:           // NOT USED
            return;
        case GDP_CMD + 5:           // DELTAX register
            gdp64_p75_out(value & 0xff);
            return;
        case GDP_CMD + 6:           // NOT USED
            return;
        case GDP_CMD + 7:           // DELTAY register
            gdp64_p77_out(value & 0xff);
            return;
        case GDP_CMD + 8:           // X register MSB
            gdp64_p78_out(value & 0xff);
            return;
        case GDP_CMD + 9:           // X register LSB
            gdp64_p79_out(value & 0xff);
            return;
        case GDP_CMD + 10:          // Y register MSB
            gdp64_p7A_out(value & 0xff);
            return;
        case GDP_CMD + 11:          // Y register LSB
            gdp64_p7B_out(value & 0xff);
            return;
        case GDP_CMD + 12:          // X-Lghtpen
        case GDP_CMD + 13:          // Y-Lightpen
        case GDP_CMD + 14:          // NOT USED
        case GDP_CMD + 15:          // NOT USED
            return;
        case PROM_DAT:
            promer_p80_out(value & 0xff);
            return;
        case PROM_A1:
            promer_p81_out(value & 0xff);
            return;
        case PROM_A2:
            promer_p82_out(value & 0xff);
            return;
        case FLO2_CMD:
            flo2_pC0_out(value & 0xff);
            return;
        case FLO2_TRACK:
            flo2_pC1_out(value & 0xff);
            return;
        case FLO2_SECT:
            flo2_pC2_out(value & 0xff);
            return;
        case FLO2_DATA:
            flo2_pC3_out(value & 0xff);
            return;
        case FLO2_ADDI:
            flo2_pC4_out(value & 0xff);
            return;
        case BANKBOOT:
            bank_pC8_out(value & 0xff);
            return;
        case CAS_CMD:
            cas_pCA_out(value & 0xff);
            return;
        case CAS_DATA:
            cas_pCB_out(value & 0xff);
            return;
        case COL_ADDR:
        case COL_JADOS_ADDR:
            col_pCC_out(value & 0xff);
            return;
        case COL_DATA:
        case COL_JADOS_DATA:
            col_pCD_out(value & 0xff);
            return;
        case COL_PAGE:
        case COL_JADOS_PAGE:
            col_pCE_out(value & 0xff);
            return;
        case UHR_DATA:
            uhr_pFE_out(value & 0xff);
            return;
        case UNKNOWN:
            return;
        default:
            log_debug("I/O byte write to UNKNOWN address %#010x = %04x", address, value);
            break;
        }
    }

    if (isRam(address))
    {
        WRITE_BYTE(g_ram, address, value);
    }
}

void cpu_write_word(unsigned int address, unsigned int value)
{
    if( g_traceFunc == false )
        g_extraSlice += (4 + (2 * g_config.numWaitStates));

    if (address > 0xffff00)
    {
        switch (address)
        {
        case GDP_CMD+2:             // X register
            gdp64_p72_out_word(value);
            return;
        case GDP_CMD+8:             // X register
            gdp64_p78_out_word(value);
            return;
        case GDP_CMD+10:            // Y register
            gdp64_p7A_out_word(value);
            return;
        default:
            log_info("I/O word write to UNKNOWN address %#010x = %06x", address, value);
            break;
        }
    }

    if (isRam(address))
    {
        // Set Word in COL256 memory

        if ((address >= g_config.col256RAMAddr) && (address <= g_config.col256RAMAddr + 0x3FFF) && g_col.col_active)
            col_setWord(address, value);

        WRITE_WORD(g_ram, address, value);
    }
}

void cpu_write_long(unsigned int address, unsigned int value)
{
    if( g_traceFunc == false )
        g_extraSlice += (8 + (4 * g_config.numWaitStates));

    if (address > 0xffff00)
    {
        switch (address)
        {
        default:
            log_debug("I/O long write to UNKNOWN address %#010x = %08x", address, value);
            break;
        }
    }

    if (isRam(address))
    {
        // Set Word in COL256 memory
        if ((address >= g_config.col256RAMAddr) && (address <= g_config.col256RAMAddr + 0x3FFF) && g_col.col_active)
            col_setLong(address, value);
        else
            WRITE_LONG(g_ram, address, value);
    }
    else
        return;
}

/* Called when the CPU pulses the RESET line */
void cpu_pulse_reset(void)
{
    g_nmi = 0;
    bank_reset();
    gdp64_reset();
    key_reset();
    col_reset();
    flo2_reset();
    cas_reset();
    ioe_reset(g_config.joystickA, g_config.joystickB);
    cent_reset();
    promer_reset();
    uhr_reset();
    sound_reset(g_config.soundDriver);
}

/* Called when the CPU changes the function code pins */
void cpu_set_fc(unsigned int fc)
{
    g_fc = fc;
}

unsigned int m68k_read_disassembler_16(unsigned int address)
{
    return cpu_read_word(address);
}

unsigned int m68k_read_disassembler_32(unsigned int address)
{
    return cpu_read_long(address);
}

void load_roms()
{
    int i, fd;
    FILE *file;

    // Bankboot ROM
    if ((file = fopen(g_config.bankBootRom, "rb")) == NULL)
        exit_error("Unable to open %s", g_config.bankBootRom);

    if ((i = fread(g_rom, 1, 0x2000, file)) == -1)
        exit_error("Error reading %s", g_config.bankBootRom);

    log_debug("Read %d bytes from Bankboot image %s.", i, g_config.bankBootRom);
    fclose(file);

    for (int idx = 0; idx < MAX_ROMS; idx++)
    {
        const char *romFile = g_config.roms[idx].path;
        if (romFile != NULL)
        {
            if ((file = fopen(romFile, "rb")) == NULL)
                exit_error("Unable to open %s", romFile);

            if ((i = fread(g_ram + g_config.roms[idx].start, 1, g_config.roms[idx].size, file)) == -1)
                exit_error("Error reading %s", romFile);
            
            if ( g_config.roms[idx].start == 0x0E0000)
                g_start_gp_ram = g_config.roms[idx].start + g_config.roms[idx].size;

            log_debug("Read %d bytes of ROM image %s.", i, romFile);
            fclose(file);
        }
    }
}

void list(int start, int end)
{
    int i;
    int pc;
    char buf[1000];

    for (pc = start; pc < end;)
    {
        i = m68k_disassemble(buf, pc, M68K_CPU_TYPE_68000);
        printf("%x %s\n", pc, buf);
        pc += i;
    }
}

void cpu_instr_callback(int pc)
{
    int diff, diff2;

    gettimeofday(&akttime, NULL);
    diff = nkc_get_diff_micros(&oldtime, &akttime);
    diff2 = nkc_get_diff_micros(&oldtime2, &akttime);
    if (diff >= 20000)
    {
        gdp64_set_vsync(1);
        //    gettimeofday(&akttime, NULL);
        gettimeofday(&oldtime, NULL);
        if(g_config.setINT == TRUE && g_config.setNMI == TRUE && g_nmi == 0 ) {
            m68k_set_irq(M68K_IRQ_7);
            g_nmi = 1;
        }
        if(g_config.setINT == TRUE && g_config.setNMI == FALSE) {
            m68k_set_irq(M68K_IRQ_5);
        }
    }
    else if (diff >= 1472 )        // 1472000 ns 
    {
        gdp64_set_vsync(0);
        g_nmi = 0;
        if(g_config.setINT == TRUE && g_config.setNMI == FALSE)
            m68k_set_irq(0);
    } 
    else    	            // As long as we are in the VSYNC period, the lower level interrupt is set
    {
        if(g_config.setINT == TRUE && g_config.setNMI == FALSE)
            m68k_set_irq(M68K_IRQ_5);
    }

    // Process events but only every 10 ms
    if (diff2 >= 10000)
    {
        handle_event();
        gui_draw();
        col_draw();
        gettimeofday(&oldtime2, NULL);
    }
}

void toggle_trace()
{
    if( g_trace == 0)
        g_trace = 1;
    else
        g_trace = 0;
    return;
}

/* The main loop */
int main(int argc, char **argv)
{
    putchar('\n');
    puts("NDR-Klein-Computer 68K Simulator V0.9");
    puts("=====================================");
    puts("based on 68000 emulator 'Musashi'");
    puts("by Martin Merck");
    putchar('\n');
    fflush(stdout);
    g_bb.bb_enabled = true;

    readConfig("./config.yaml");
    init_windows(true);

    cas_setFile(g_config.casFile);
    cent_setFile(g_config.listFile);
    promer_setFile(g_config.promFile);

    load_roms();

    // nkc
    m68k_init();
    m68k_set_cpu_type(M68K_CPU_TYPE_68000);

    m68k_pulse_reset();
    cpu_pulse_reset();
    //nmi_device_reset();
    gettimeofday(&oldtime, NULL);
    gettimeofday(&oldtime2, NULL);
    gettimeofday(&akttime, NULL);

    struct timeval starttime;
    struct timeval endtime;

    struct timespec start;
    struct timespec end;
    long long simNanos = 0;
    long long realNanos = 0;

    struct timespec res;

    long long extraNanos = 0;
    
    clock_gettime( CLOCK_REALTIME, &start);
    while (true)
    {
        int slices;

        if (g_trace)
        {
            trace();
            SDL_Delay(100);
        }

        //    m68k_execute(g_trace ? 1 : 1000); // execute 10,000 MC68000 instructions
        if (!g_gdp.isGuiScreen) // Stop Simulation if GUI screen
        {
            if( g_trace == true)
            {
                slices = m68k_execute(1); // execute 1 MC68000 instructions
            } else {
                slices = m68k_execute(10000); // execute 10,000 MC68000 cpu cylcles ()
            }

            long long motorolaNanos = (slices + g_extraSlice) * (1000 / g_config.cpuSpeed);
            g_extraSlice = 0;
            realNanos += motorolaNanos;
            clock_gettime( CLOCK_REALTIME, &end);
            long long elapsedNanos = nkc_get_diff_nanos(&start, &end);

            if(elapsedNanos > motorolaNanos)
            {
                extraNanos += elapsedNanos - motorolaNanos;
            }
            else
            {
                if(g_config.simSpeed != 0) {
                    long long sleepNanos = motorolaNanos - elapsedNanos - extraNanos;
                    if( sleepNanos < 0) {
                        extraNanos += sleepNanos;
                    } else {
                        nkc_sleep_nanos(sleepNanos);
                        extraNanos = 0;
                    }
                }
            }
            clock_gettime( CLOCK_REALTIME, &end);
            elapsedNanos = nkc_get_diff_nanos(&start, &end);
            clock_gettime( CLOCK_REALTIME, &start);
            simNanos += elapsedNanos;
        }
        else
        {
            cpu_instr_callback(0); /// but call callback to handle screen and events
        }

        if( simNanos > 10000000000 )
        {
            double speed = (double) realNanos / (double) simNanos;
            realNanos = 0;
            simNanos = 0;

            log_info("Simulated CPU Speed (MHz): %4.2lf",speed * g_config.cpuSpeed);
        }
    }
    return 0;
}
