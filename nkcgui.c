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
 * GUI windw for controls of the simulation.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include "68k-nkcemu.h"
#include "gdp64.h"
#include "key.h"
#include "config.h"
#include "nkcgui.h"
#include "gui_file.h"
#include "gui_group.h"
#include "gui_button.h"
#include "directory.h"
#include "sound.h"
#include "ioe.h"
#include "util.h"
#include "log.h"

gui g_gui;

extern config g_config;
extern gdp64 g_gdp;
extern key g_key;
extern sound g_sound;
extern ioe g_ioe;
extern file_status g_file_stat;


void gui_quit() 
{
    TTF_Quit();
    IMG_Quit();
    if(g_gui.renderer != NULL)
        SDL_DestroyRenderer(g_gui.renderer);
    if(g_gui.window != NULL)
        SDL_DestroyWindow(g_gui.window);
    SDL_Quit();
    exit(1);
}

int gui_init(void)
{

//    int rendererFlags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;
    int rendererFlags = SDL_RENDERER_SOFTWARE | SDL_RENDERER_TARGETTEXTURE;

#if defined(__linux__)    // With Wayland, windows can not be positioned, so need to have a border  
    int windowFlags = 0;
#else
    int windowFlags = SDL_WINDOW_BORDERLESS;
#endif

    double scale = (512. * g_gdp.xmag) / SCREEN_WIDTH; 
    g_gui.window = SDL_CreateWindow( "NDR-NKC", 
                                     200, 
                                     100 + 256 * g_gdp.ymag,
                                     512 * g_gdp.xmag,
                                     (int) rint(SCREEN_HEIGHT * scale),
                                     windowFlags );
	if (!g_gui.window)
	{
    	SDL_LogMessage( SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR,
                        "Failed to open GUI window: %s",
                        SDL_GetError() );
		exit(1);
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	g_gui.renderer = SDL_CreateRenderer(g_gui.window, -1, rendererFlags);
	if (!g_gui.renderer)
	{
    	SDL_LogMessage( SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR,
                        "Failed to create renderer: %s",
                        SDL_GetError() );
		exit(1);
	}
    
    SDL_SetRenderDrawColor( g_gui.renderer, 0, 0, 0, 255 );
    SDL_RenderClear( g_gui.renderer );
    SDL_RenderPresent( g_gui.renderer );

    //Initialize image loading
    int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;

    if( !( IMG_Init( imgFlags ) & imgFlags ) )
    {
        SDL_LogMessage( SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR,
                        "SDL_image could not initialize! SDL_image Error: %s\n",
                        IMG_GetError() );
        gui_quit();
    }

    //Load media
    g_gui.nkc_texture = IMG_LoadTexture(g_gui.renderer, "./resources/images/NKC.png");
    if ( g_gui.nkc_texture == NULL )
        log_warn("Could not open ./resources/images/NKC.png");
    g_gui.floppy_texture = IMG_LoadTexture(g_gui.renderer, "./resources/images/Floppy_LED.png");
    if ( g_gui.floppy_texture == NULL )
        log_warn("Could not open ./resources/images/Floppy_LED.png");
    g_gui.green_leds_texture = IMG_LoadTexture(g_gui.renderer, "./resources/images/GLeds-on.JPG");
    if ( g_gui.green_leds_texture == NULL )
        log_warn("Could not open ./resources/images/GLeds-on.JPG");
    g_gui.red_leds_texture = IMG_LoadTexture(g_gui.renderer, "./resources/images/RLeds-on.JPG");
    if ( g_gui.red_leds_texture == NULL )
        log_warn("Could not open ./resources/images/RLeds-on.JPG");
    g_gui.toggle_off_texture = IMG_LoadTexture(g_gui.renderer, "./resources/images/Toggle-off.JPG");
    if ( g_gui.toggle_off_texture == NULL )
        log_warn("Could not open ./resources/images/Toggle-off.JPG");
    g_gui.toggle_on_texture = IMG_LoadTexture(g_gui.renderer, "./resources/images/Toggle-on.JPG");
    if ( g_gui.toggle_on_texture == NULL )
        log_warn("Could not open ./resources/images/Toggle-on.JPG");
    g_gui.push_texture = IMG_LoadTexture(g_gui.renderer, "./resources/images/Push.JPG");
    if ( g_gui.push_texture == NULL )
        log_warn("Could not open ./resources/images/Push.JPG");

    //Initialize SDL_ttf
    if( TTF_Init() == -1 )
    {
        SDL_LogMessage( SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
		gui_quit();
    }
       //Open the font
    g_gui.font = TTF_OpenFont( "./resources/fonts/OpenSans-Medium.ttf", 11 );
    if( g_gui.font == NULL )
    {
        SDL_LogMessage( SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_ERROR, "Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError() );
		gui_quit();
    }

    scale = (512. * g_gdp.xmag) / SCREEN_WIDTH; 
    g_gui.gui_buttons[BUTTON_PROM]     = button_new( 124, 42, 46, 95 );
    g_gui.gui_buttons[BUTTON_CAS]      = button_new( 199, 20, 151, 96 );
    g_gui.gui_buttons[BUTTON_CENT]     = button_new( 196, 143, 152, 39 );
    g_gui.gui_buttons[BUTTON_SOUND]    = button_new( 363, 20, 98, 96 );
    g_gui.gui_buttons[BUTTON_FLOPPY_A] = button_new( 491, 20, 263, 70 );
    g_gui.gui_buttons[BUTTON_FLOPPY_B] = button_new( 491, 128, 263, 70 );
    g_gui.gui_buttons[BUTTON_JOY_A]    = button_new( 975, 20, 61, 79 );
    g_gui.gui_buttons[BUTTON_JOY_B]    = button_new( 975, 124, 61, 79 );
    g_gui.gui_buttons[BUTTON_RESET]    = button_new( 363, 184, 30, 30 );
    g_gui.gui_buttons[BUTTON_PASTE]   = button_new( 433, 184, 30, 30 );
    g_gui.gui_buttons[BUTTON_TURBO]    = button_new( 363, 134, 30, 30 );
    g_gui.gui_buttons[BUTTON_INT]      = button_new( 398, 134, 30, 30 );
    g_gui.gui_buttons[BUTTON_NMI]      = button_new( 433, 134, 30, 30 );

    g_gui.current_path = (char *) malloc( FILENAME_MAX + 1 );
    getcwd(g_gui.current_path, FILENAME_MAX);

    return SDL_GetWindowID( g_gui.window );
}

void gui_draw_text(const char * text, int x, int y)
{
	SDL_Surface *surf;
    SDL_Texture *tex;
    SDL_Color color;
    color.b=0;
    color.g=0;
    color.r=0;
    SDL_Rect rect;
    surf = TTF_RenderText_Solid(g_gui.font, text, color);
    tex = SDL_CreateTextureFromSurface(g_gui.renderer, surf);
    rect.x=x;
    rect.y=y;
    rect.w=surf->w;
    rect.h=surf->h;
    SDL_FreeSurface(surf);
    SDL_RenderCopy(g_gui.renderer,tex,NULL,&rect);
    SDL_DestroyTexture(tex);
}

void gui_draw(void)
{
	SDL_SetRenderDrawColor(g_gui.renderer, 200, 200, 200, 255);
	SDL_RenderClear(g_gui.renderer);

    SDL_Texture* back = SDL_CreateTexture(g_gui.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, 1066, 234);
    //change the rendering target

    SDL_SetTextureBlendMode(back, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(g_gui.renderer, back);

	SDL_Rect dest;

	dest.x = 0;
	dest.y = 0;
    dest.w = 1066.;
    dest.h = 234.;

	SDL_RenderCopy(g_gui.renderer, g_gui.nkc_texture, NULL, &dest);
 
 	SDL_Rect led_src, led_dest;
    BYTE_68K ioe_porta = ioe_get_p30();
    BYTE_68K ioe_portb = ioe_get_p31();
    for ( int i=0; i<8; i++ ) {
        int j = 1 << i;
        if( (ioe_porta & j) != 0 )
        {
            led_src.x = (7-i) * 200;
            led_src.y = 0;
            led_src.w = 200;
            led_src.h = 195;

            led_dest.x = 787 + (7-i) * 22;
            led_dest.y = 48;
            led_dest.w = 22;
            led_dest.h = 22;

            SDL_RenderCopy(g_gui.renderer, g_gui.green_leds_texture, &led_src, &led_dest);
        }
        if( (ioe_portb & j) != 0 )
        {
            led_src.x = (7-i) * 200;
            led_src.y = 0;
            led_src.w = 200;
            led_src.h = 238;

            led_dest.x = 786 + (7-i) * 22;
            led_dest.y = 150;
            led_dest.w = 22;
            led_dest.h = 24;

            SDL_RenderCopy(g_gui.renderer, g_gui.red_leds_texture, &led_src, &led_dest);
        }
    }

    double scale = (512. * g_gdp.xmag) / SCREEN_WIDTH; 
    SDL_Rect toggle_src, toggle_dest;
    toggle_src.x = 0;
    toggle_src.y = 0;
    toggle_src.w = 30;
    toggle_src.h = 30;
	toggle_dest.x = (int) rint(g_gui.gui_buttons[BUTTON_TURBO]->rect.x / scale);
	toggle_dest.y = (int) rint(g_gui.gui_buttons[BUTTON_TURBO]->rect.y / scale);
    toggle_dest.w = (int) rint(g_gui.gui_buttons[BUTTON_TURBO]->rect.w / scale);
    toggle_dest.h = (int) rint(g_gui.gui_buttons[BUTTON_TURBO]->rect.h / scale);
    if( g_config.simSpeed == 0 )
        SDL_RenderCopy(g_gui.renderer, g_gui.toggle_on_texture, &toggle_src, &toggle_dest);
    else
        SDL_RenderCopy(g_gui.renderer, g_gui.toggle_off_texture, &toggle_src, &toggle_dest);

	toggle_dest.x = (int) rint(g_gui.gui_buttons[BUTTON_INT]->rect.x / scale);
	toggle_dest.y = (int) rint(g_gui.gui_buttons[BUTTON_INT]->rect.y / scale);
    toggle_dest.w = (int) rint(g_gui.gui_buttons[BUTTON_INT]->rect.w / scale);
    toggle_dest.h = (int) rint(g_gui.gui_buttons[BUTTON_INT]->rect.h / scale);
    if( g_config.setINT == FALSE )
        SDL_RenderCopy(g_gui.renderer, g_gui.toggle_off_texture, &toggle_src, &toggle_dest);
    else
        SDL_RenderCopy(g_gui.renderer, g_gui.toggle_on_texture, &toggle_src, &toggle_dest);

	toggle_dest.x = (int) rint(g_gui.gui_buttons[BUTTON_NMI]->rect.x / scale);
	toggle_dest.y = (int) rint(g_gui.gui_buttons[BUTTON_NMI]->rect.y / scale);
    toggle_dest.w = (int) rint(g_gui.gui_buttons[BUTTON_NMI]->rect.w / scale);
    toggle_dest.h = (int) rint(g_gui.gui_buttons[BUTTON_NMI]->rect.h / scale);
    if( g_config.setNMI == FALSE )
        SDL_RenderCopy(g_gui.renderer, g_gui.toggle_off_texture, &toggle_src, &toggle_dest);
    else
        SDL_RenderCopy(g_gui.renderer, g_gui.toggle_on_texture, &toggle_src, &toggle_dest);

	toggle_dest.x = (int) rint(g_gui.gui_buttons[BUTTON_RESET]->rect.x / scale);
	toggle_dest.y = (int) rint(g_gui.gui_buttons[BUTTON_RESET]->rect.y / scale);
    toggle_dest.w = (int) rint(g_gui.gui_buttons[BUTTON_RESET]->rect.w / scale);
    toggle_dest.h = (int) rint(g_gui.gui_buttons[BUTTON_RESET]->rect.h / scale);
    SDL_RenderCopy(g_gui.renderer, g_gui.push_texture, &toggle_src, &toggle_dest);
	toggle_dest.x = (int) rint(g_gui.gui_buttons[BUTTON_PASTE]->rect.x / scale);
	toggle_dest.y = (int) rint(g_gui.gui_buttons[BUTTON_PASTE]->rect.y / scale);
    toggle_dest.w = (int) rint(g_gui.gui_buttons[BUTTON_PASTE]->rect.w / scale);
    toggle_dest.h = (int) rint(g_gui.gui_buttons[BUTTON_PASTE]->rect.h / scale);
    SDL_RenderCopy(g_gui.renderer, g_gui.push_texture, &toggle_src, &toggle_dest);

    // For debugging we draw the button rectangles
    //
    // for( int i = 0; i < GUI_NUM_BUTTONS; i++) 
    // {
    //     SDL_Rect button_rect;
    //     button_rect.x = (int) rint(g_gui.gui_buttons[i]->rect.x / scale);
    //     button_rect.y = (int) rint(g_gui.gui_buttons[i]->rect.y / scale);
    //     button_rect.w = (int) rint(g_gui.gui_buttons[i]->rect.w / scale);
    //     button_rect.h = (int) rint(g_gui.gui_buttons[i]->rect.h / scale);
    //     SDL_SetRenderDrawColor(g_gui.renderer, 0, 0, 0, 255);
    //     SDL_RenderDrawRect(g_gui.renderer, &button_rect);
    // }

 	dest.x = 0;
	dest.y = 0;
    dest.w = (int) rint(SCREEN_WIDTH * scale);
    dest.h = (int) rint(SCREEN_HEIGHT * scale);

    SDL_SetRenderTarget(g_gui.renderer, NULL); //NULL SETS TO DEFAULT
    SDL_RenderCopy(g_gui.renderer, back, NULL, &dest);
    SDL_DestroyTexture(back);

    char str[80];
    if( g_config.casFile != NULL )
        gui_draw_text(nkc_get_filename(g_config.casFile),190,115);
    if( g_config.listFile != NULL )
        gui_draw_text(nkc_get_filename(g_config.listFile),190,180);
    if( g_config.promFile != NULL )
        gui_draw_text(nkc_get_filename(g_config.promFile),120,190);
    if( g_config.diskA != NULL )
        gui_draw_text(nkc_get_filename(g_config.diskA),470,90);
    if( g_config.diskB != NULL )
        gui_draw_text(nkc_get_filename(g_config.diskB),470,193);
    if( g_config.soundDriver != NULL )
    {
        sprintf(str,"%.15s",g_config.soundDriver);
        gui_draw_text(str,347,115);
    }
    gui_draw_text("IOE Port A ($FFFFFF30)",755,27);
    if( g_config.joystickA != NULL )
    {
        sprintf(str,"%.25s",g_config.joystickA);
        gui_draw_text(str,755,70);
    }
    gui_draw_text("IOE Port B ($FFFFFF31)",755,125);
    if( g_config.joystickB != NULL )
    {
        sprintf(str,"%.25s",g_config.joystickB);
        gui_draw_text(str,755,170);
    }
    gui_draw_text("Reset",rint(363 * scale), rint(214 * scale));
    gui_draw_text("Paste",rint(433 * scale), rint(214 * scale));
    gui_draw_text("Turbo",rint(363 * scale), rint(164 * scale));
    gui_draw_text("  INT",rint(398 * scale), rint(164 * scale));
    gui_draw_text(" NMI",rint(433 * scale), rint(164 * scale));
    SDL_RenderPresent(g_gui.renderer);
}

void gui_event(SDL_Event* event)
{
    SDL_Event* evt = event;
   	if (event->type == SDL_MOUSEMOTION ) {
        Uint32 flags = SDL_GetWindowFlags(g_gui.window);
        //if (( flags & SDL_WINDOW_MOUSE_FOCUS ) == 0)
            SDL_RaiseWindow(g_gui.window);
    }
   	if (event->type == SDL_MOUSEBUTTONDOWN ) {
		log_debug("Button pressed %d %d",event->button.x, event->button.y);
        int button = -1;
        nkc_directory* dir;
        for( int i = 0; i < GUI_NUM_BUTTONS; i++) 
        {
            if( button_event(g_gui.gui_buttons[i], event->button.x, event->button.y) )
                button = i;
        }
        gdp64_save_regs();
        char* normalized_path, *path;
        switch (button)
        {
            case BUTTON_PROM:
                log_debug("Promer button pressed");
                g_gui.active_dialog = GUI_PROM;
                normalized_path = nkc_normalize_path(g_gui.current_path, g_config.promFile);
                path = nkc_remove_last(normalized_path);
                free(normalized_path);
                g_file_stat.filter = ".bin";
                dir = dir_listfiles(path);
                file_new(dir, "Auswahl PROM image");
                break;
            case BUTTON_CAS:
                log_debug("Cas button pressed");
                g_gui.active_dialog = GUI_CAS;
                normalized_path = nkc_normalize_path(g_gui.current_path, g_config.casFile);
                path = nkc_remove_last(normalized_path);
                free(normalized_path);
                g_file_stat.filter = ".cas";
                dir = dir_listfiles(path);
                file_new(dir, "Auswahl Cassette");
                break;
            case BUTTON_CENT:
                log_debug("Centronics button pressed");
                g_gui.active_dialog = GUI_CENT;
                normalized_path = nkc_normalize_path(g_gui.current_path, g_config.listFile);
                path = nkc_remove_last(normalized_path);
                free(normalized_path);
                g_file_stat.filter = ".lst";
                dir = dir_listfiles(path);
                file_new(dir, "Auswahl Listing Datei");
                break;
            case BUTTON_SOUND:
                log_debug("Sound button pressed");
                g_gui.active_dialog = GUI_SOUND;
                group_new(g_sound.devices, "Auswahl Sound-Treiber");
                break;
            case BUTTON_FLOPPY_A:
                log_debug("Floppy A button pressed");
                g_gui.active_dialog = GUI_FLOPPY_A;
                normalized_path = nkc_normalize_path(g_gui.current_path, g_config.diskA);
                path = nkc_remove_last(normalized_path);
                free(normalized_path);
                g_file_stat.filter = ".img";
                dir = dir_listfiles(path);
                file_new(dir, "Floppy A - Auswahl");
                break;
            case BUTTON_FLOPPY_B:
                log_debug("Floppy B button pressed");
                g_gui.active_dialog = GUI_FLOPPY_B;
                normalized_path = nkc_normalize_path(g_gui.current_path, g_config.diskA);
                path = nkc_remove_last(normalized_path);
                free(normalized_path);
                g_file_stat.filter = ".img";
                dir = dir_listfiles(path);
                file_new(dir, "Floppy B - Auswahl");
                break;
            case BUTTON_JOY_A:
                log_debug("Joystick A button pressed");
                g_gui.active_dialog = GUI_JOY_A;
                group_new(g_ioe.devices, "Auswahl Joystick Port 30");
                break;
            case BUTTON_JOY_B:
                log_debug("Joystick B button pressed");
                g_gui.active_dialog = GUI_JOY_B;
                group_new(g_ioe.devices, "Auswahl Joystick Port 31");
                break;
            case BUTTON_RESET:
                log_debug("Reset button pressed");
                nkc_reset();
                break;
            case BUTTON_PASTE:
                log_debug("Insert button pressed");
                if (SDL_HasClipboardText() == SDL_TRUE)
                {
                    if (g_key.clipboardText != NULL)
                    {
                        SDL_free(g_key.clipboardText);
                    }
                    g_key.clipboardText = SDL_GetClipboardText();
                    g_key.clipboardLength = strlen(g_key.clipboardText);
                    g_key.clipboardOffset = 0;
                }
                break;
            case BUTTON_TURBO:
                log_debug("Turbo button pressed");
                if( g_config.simSpeed == 0 )
                    g_config.simSpeed = 1;
                else
                    g_config.simSpeed = 0;
                break;
            case BUTTON_INT:
                log_debug("INT button pressed");
                if( g_config.setINT == FALSE )
                    g_config.setINT = TRUE;
                else
                    g_config.setINT = FALSE;
                // toggle_trace();
                break;
            case BUTTON_NMI:
                log_debug("NMI button pressed");
                if( g_config.setNMI == FALSE )
                    g_config.setNMI = TRUE;
                else
                    g_config.setNMI = FALSE;
                break;
            default:
                break;
        }
    }

    if (event->type == SDL_KEYDOWN)
    {
        int key = nkc_get_ascii(event->key);

        if( g_gui.active_dialog == GUI_SOUND ||
            g_gui.active_dialog == GUI_JOY_A ||
            g_gui.active_dialog == GUI_JOY_B )
            group_select_event(key);

        if( g_gui.active_dialog == GUI_PROM ||
            g_gui.active_dialog == GUI_CAS ||
            g_gui.active_dialog == GUI_CENT ||
            g_gui.active_dialog == GUI_FLOPPY_A ||
            g_gui.active_dialog == GUI_FLOPPY_B )
            file_select_event(key);
	}

	return;
}
