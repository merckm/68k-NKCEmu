
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
#include "mouse.h"
#include <stdbool.h>
#include "gdp64.h"

#define KEY_OFS_ADDR   3u   // 0x8B
#define DOWN_OFS_ADDR  4u   // 0x8C
#define UP_OFS_ADDR    5u   // 0x8D -> copy cnt to regs
#define RIGHT_OFS_ADDR 6u   // 0x8E -> Clear cnt
#define LEFT_OFS_ADDR  7u   // 0x8F

static unsigned int cnt_up=0, cnt_down=0, cnt_right=0, cnt_left=0;
static mouse_registers regs = {.key_stat=0xFF, .down_reg=0,.left_reg=0, .right_reg=0, .up_reg=0};
typedef struct {
    unsigned int x;
    unsigned int y;
} mouse_xy_t;

static mouse_xy_t mouse_xy={0};
static mouse_xy_t last_mouse_xy={.x=GDP_X_RES/2,.y=GDP_Y_RES/2};
//

void mouse_event(SDL_Event *event)
{
    static const BYTE key_map[]={0x80,0x20,0x40};
    switch(event->type) {
    //if (event->type == SDL_MOUSEMOTION ) {
        case SDL_MOUSEMOTION:
            mouse_xy.x = event->motion.x/get_x_mag();
            mouse_xy.y = (GDP_Y_RES - 1) - event->motion.y/get_y_mag();

            //printf("%u.%u\n",mouse_xy.x,mouse_xy.y);
            const int dx = (int)mouse_xy.x - (int)last_mouse_xy.x;
            if(dx<0) {
                cnt_left += (abs(dx)); //*8)/10; ///2;
            }else{
                cnt_right += (dx); //*8)/10; ///2;
            }
            const int dy = (int)mouse_xy.y - (int)last_mouse_xy.y;
            if(dy<0) {
                cnt_up += abs(dy); ///2;
            }else{
                cnt_down += dy; ///2;
            }
            last_mouse_xy = mouse_xy;
            //printf("%u.%u->%u %u %u %u Button:0x%X\n",mouse_xy.x,mouse_xy.y,cnt_left, cnt_right, cnt_up, cnt_down, event->motion.state);

            break;
        case SDL_MOUSEBUTTONDOWN:
            //printf("Mousebutton down: 0x%X\n", event->button.button);
            {
                const BYTE key = event->button.button - 1;
                if(key<sizeof(key_map)) {
                    regs.key_stat &= ~key_map[key];
                }
            }
            break;
        case SDL_MOUSEBUTTONUP:
            //printf("Mousebutton up: 0x%X\n", event->button.button);
            {
                const BYTE key = event->button.button - 1;
                if(key<sizeof(key_map)) {
                    regs.key_stat |= key_map[key];
                }
            }
            break;
    }
}

BYTE bus_mouse_in(const BYTE ofs)
{
    BYTE ret =0u;
    switch(ofs) {
        case KEY_OFS_ADDR:
            ret = regs.key_stat;
            //printf("Keys: 0x%X\n",ret);
            break;
        case DOWN_OFS_ADDR:
            ret = regs.down_reg;
            break;
        case UP_OFS_ADDR:
            ret = regs.up_reg;
            break;
        case LEFT_OFS_ADDR:
            ret = regs.left_reg;
            break;
        case RIGHT_OFS_ADDR:
            ret = regs.right_reg;
            break;
    }
    return ret;
}

void bus_mouse_out(const BYTE ofs, const BYTE data)
{
    switch(ofs) {
        case RIGHT_OFS_ADDR:
            cnt_up = cnt_down = cnt_right = cnt_left = 0u;
            break;
        case UP_OFS_ADDR:
            regs.up_reg    = (BYTE)cnt_up;
            regs.down_reg  = (BYTE)cnt_down;
            regs.left_reg  = (BYTE)cnt_left;
            regs.right_reg = (BYTE)cnt_right;
            break;
        default:
            // Nothing to do
            break;
    }
}
