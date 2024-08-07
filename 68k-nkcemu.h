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

#ifndef SIM__HEADER
#define SIM__HEADER

/* Memory-mapped IO ports */

/* Simulated NKC hardware components
 * Key, GDP64K, COL256, Flo2, CAS, Promer, IOE, Cent, Uhr, Bankboot, Sound
 */
#define CPU 1
#define IOE_PORT_A (CPU * 0x00ffff30L)       // IOE Port A
#define IOE_PORT_B (CPU * 0x00ffff31L)       // IOE Port B
#define SOUND_ADR (CPU * 0x00ffff40L)        // sound address register
#define SOUND_DATA (CPU * 0x00ffff41L)       // sound data register
#define SOUND_JADOS_ADR (CPU * 0x00ffff50L)  // sound address register for JADOS
#define SOUND_JADOS_DATA (CPU * 0x00ffff51L) // sound data register for JADOS
#define CENT_DAT (CPU * 0x00ffff48L)         // Cent data register
#define CENT_IN (CPU * 0x00ffff49L)          // Cent command/status register
#define CENT_STB (CPU * 0x00ffff49L)         // Cent read strobe = command/status register
#define KEY_DATA (CPU * 0x00ffff68L)         // Key status/data keyboard register
#define KEY_DIP (CPU * 0x00ffff69L)          // Key data DIP switches register
#define GDP_PAGE (CPU * 0x00ffff60L)         // GDP page register
#define GDP_SCROLL (CPU * 0x00ffff61L)       // GDP HardscrolL) register
#define GDP_CMD (CPU * 0x00ffff70L)          // Basis of GDP registers
#define PROM_DAT (CPU * 0x00ffff80L)         // Promer data register
#define PROM_A1 (CPU * 0x00ffff81L)          // Promer address LSB register
#define PROM_A2 (CPU * 0x00ffff82L)          // Promer address MSB register
#define MOUSE_BASE (CPU * 0x00ffff88L)
#define MOUSE_KEY (CPU * 0x00ffff8BL)        // Mouse Key
#define MOUSE_DOWN (CPU * 0x00ffff8CL)       // Mouse Down register
#define MOUSE_UP (CPU * 0x00ffff8DL)         // Mouse Up register
#define MOUSE_RIGHT (CPU * 0x00ffff8EL)      // Mouse Right register
#define MOUSE_LEFT (CPU * 0x00ffff8FL)      // Mouse left register
#define COLOR_A0 (CPU * 0x00ffffA0L)         // Forground COLOR used by GRUND
#define COLOR_A1 (CPU * 0x00ffffA1L)         // Background COLOR used by GRUND
#define FLO2_CMD (CPU * 0x00ffffC0L)         // FLO2 Command/status register
#define FLO2_TRACK (CPU * 0x00ffffC1L)       // FLO2 Track register
#define FLO2_SECT (CPU * 0x00ffffC2L)        // FLO2 Sector register
#define FLO2_DATA (CPU * 0x00ffffC3L)        // FLO2 Data register
#define FLO2_ADDI (CPU * 0x00ffffC4L)        // FLO2 Zusatz register
#define BANKBOOT (CPU * 0x00ffffC8L)         // BankBoot register
#define CAS_CMD (CPU * 0x00ffffCAL)          // Cas command/status register
#define CAS_DATA (CPU * 0x00ffffCBL)         // Cas data register
#define COL_ADDR (CPU * 0x00ffffCCL)         // Col256 address register
#define COL_DATA (CPU * 0x00ffffCDL)         // Col256 data register
#define COL_PAGE (CPU * 0x00ffffCEL)         // Col256 memory page selection
#define COL_JADOS_ADDR (CPU * 0x00ffffACL)         // Col256 address register
#define COL_JADOS_DATA (CPU * 0x00ffffADL)         // Col256 data register
#define COL_JADOS_PAGE (CPU * 0x00ffffAEL)         // Col256 memory page selection
#define UHR_DATA (CPU * 0x00ffffFEL)         // Uhr serial port bits
#define SER_DATA  (CPU * 0x00ffffF0L)           // SER RX/TX data
#define SER_STATUS (CPU * 0x00ffffF1L)           // SER Status Register
#define SER_COMMAND (CPU * 0x00ffffF2L)          // SER Command Register
#define SER_CONTROL (CPU * 0x00ffffF3L)          // SER Control Register
#define FPGA_TIMER_BASE (CPU * 0x00ffffF4L)   // FPGA-Timer
#define FPGA_TIMER_CTRL (CPU * 0x00ffffF4L)   // FPGA-Timer control register
#define FPGA_TIMER_TRH (CPU * 0x00ffffF5L)   // FPGA-Timer timer/reload register high
#define FPGA_TIMER_TRL (CPU * 0x00ffffF6L)   // FPGA-Timer timer/reload register low
#define UNKNOWN (CPU * 0x00ffffffL)          // used during memory scan, will ignore

/* ROM and RAM sizes */
#define MAX_BBROM 0x001fff // 8 KB ROM
#define MAX_RAM 0x0fffff   // 1 MB of address space on 68008

#ifdef __cplusplus
extern "C"
{
#endif

    void nkc_reset(void);
    unsigned int cpu_read_byte(unsigned int address);
    unsigned int cpu_read_word(unsigned int address);
    unsigned int cpu_read_long(unsigned int address);
    void cpu_write_byte(unsigned int address, unsigned int value);
    void cpu_write_word(unsigned int address, unsigned int value);
    void cpu_write_long(unsigned int address, unsigned int value);
    void cpu_pulse_reset(void);
    void cpu_set_fc(unsigned int fc);
    void cpu_instr_callback(int pc);
    void toggle_trace();

#ifdef __cplusplus
}
#endif

#endif /* SIM__HEADER */
