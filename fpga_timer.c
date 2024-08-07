
/**************************************************************************************
 *   Copyright (C) 2023,2024 by Martin Merck, Andreas Voggeneder                      *
 *   martin.merck@gmx.de                                                              *
 *   andreas_v@gmx.at                                                                 *
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
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include "fpga_timer.h"
#include <stdbool.h>
#include <SDL_timer.h>
#include "gdp64.h"
#include "m68k.h"

#define CTRL_OFS_ADDR  0u   // 0xF8
#define TRH_OFS_ADDR   1u   // 0xF5
#define TRL_OFS_ADDR   2u   // 0xF6

// CTRL-Register bitfields
#define CTRL_RUN_MASK 0x01u
#define CTRL_WRM_MASK 0x06u
#define CTRL_WRM_OFS  1u
#define CTRL_TOVF_MASK 0x40
#define CTRL_IE_MASK  0x80u

static fpga_timer_registers regs = {0};
static bool timer_initialized=false;
static SDL_TimerID myTimer={0};
//static int cnt=0;
static uint32_t last_tick=0u;

uint32_t callback(uint32_t interval, void* name) {
    const uint16_t delta = (regs.reload+500)/1000;
    /*if(!cnt) {
        cnt=10;
        printf("Timer %u!\n", interval);
    }else{
        cnt--;
    }*/
    m68k_set_irq(M68K_IRQ_5);
    regs.ctrl |= CTRL_TOVF_MASK;
    const uint32_t current_tick=SDL_GetTicks();
    uint32_t new_interval = interval;
    if(last_tick>0u) {
        last_tick +=delta;
        if (last_tick > current_tick) {
            new_interval = last_tick - current_tick;
            printf("Correct: %u\n",new_interval);
        }else{
            last_tick=current_tick;
        }
        //printf("Timer %u %u %u\n", last_tick+delta, current_tick, current_tick-last_tick);
    }

    if ((regs.ctrl & CTRL_RUN_MASK)) {
        return new_interval;
    }else{
        return 0;   // Stop Timer
    }
}

// Read-access
BYTE bus_fpga_timer_in(const BYTE ofs)
{
    BYTE ret =0u;
    switch(ofs) {
        case CTRL_OFS_ADDR:
            ret = regs.ctrl;
            //printf("Keys: 0x%X\n",ret);
            break;
        case TRH_OFS_ADDR:
            ret = (regs.timer>>8) & 0xFF;
            break;
        case TRL_OFS_ADDR:
            ret = (regs.timer) & 0xFF;
            break;
        default:
            // Nothing to do
            break;
    }
    return ret;
}

// Write Access
void bus_fpga_timer_out(const BYTE ofs, const BYTE data)
{
    static BYTE temp=0;
    //printf("bus_fpga_timer_out: 0x%X=0x%X\n",ofs,data);
    switch(ofs) {
        case CTRL_OFS_ADDR:
            regs.ctrl = data;
            if (((regs.ctrl & CTRL_RUN_MASK)==0) && (timer_initialized)) {
                timer_initialized=false;
                SDL_RemoveTimer(myTimer);
                last_tick = 0u;
                printf("Timer stopped\n");
            }
            break;
        case TRH_OFS_ADDR:
            temp = data;
            break;
        case TRL_OFS_ADDR:
            if ((((regs.ctrl & CTRL_WRM_MASK)>>1) == 0) || (((regs.ctrl & CTRL_WRM_MASK)>>CTRL_WRM_OFS) == 2)) {
                regs.reload =  ((uint16_t)temp << 8) | ((uint16_t)data);
                printf("Reload: 0x%04X\n",regs.reload);
                if (timer_initialized) {
                    timer_initialized=false;
                    SDL_RemoveTimer(myTimer);
                    last_tick =0;
                }
                if (regs.reload>=1000) {
                    myTimer = SDL_AddTimer((regs.reload+500)/1000, callback, NULL);
                    timer_initialized=true;
                    last_tick = SDL_GetTicks();
                    printf("Timer initialized with %d ms\n", (regs.reload+500)/1000);
                }
            }
            if ((((regs.ctrl & CTRL_WRM_MASK)>>1) == 1) || (((regs.ctrl & CTRL_WRM_MASK)>>CTRL_WRM_OFS) == 2)) {
                regs.timer =  ((uint16_t)temp << 8) | ((uint16_t)data);
                printf("Timer: 0x%04X\n",regs.timer);
            }
            break;
        default:
            // Nothing to do
            break;
    }
}


