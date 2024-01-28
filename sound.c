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
 * Emulates a Sound interface with a General Instruments AY-3-8912 sound chip.
 * It utilizes the AYUMI highly precise emulation of the AY-3-8910 and YM2149 sound chips
 * written by Peter Sovietov (GITHub: https://github.com/true-grue/ayumi,
 * Web: http://sovietov.com/app/ayumi/ayumi.html).
 */

#include <stdio.h>
#include <unistd.h>
#include <SDL.h>
#include <SDL_audio.h>
#include "log.h"
#include "sound.h"

sound g_sound;

enum Register
{
    AY_AFINE = 0,
    AY_ACOARSE = 1,
    AY_BFINE = 2,
    AY_BCOARSE = 3,
    AY_CFINE = 4,
    AY_CCOARSE = 5,
    AY_NOISEPER = 6,
    AY_ENABLE = 7,
    AY_AVOL = 8,
    AY_BVOL = 9,
    AY_CVOL = 10,
    AY_EFINE = 11,
    AY_ECOARSE = 12,
    AY_ESHAPE = 13,
    AY_PORTA = 14,
    AY_PORTB = 15
};

// Get current Address
BYTE sound_p40_in()
{
    return g_sound.ayAddress;
}

// Set Address
void sound_p40_out(BYTE data)
{
    g_sound.ayAddress = data;
    log_debug("Writing SOUND-Address %2X", data);
}

BYTE sound_p41_in()
{
    printf("Reading SOUND-Data from address %2X", g_sound.ayAddress);
    BYTE rc = 0xff;
    switch (g_sound.ayAddress)
    {
    case AY_AFINE:
        rc = (BYTE)(g_sound.toneA & 0x00FF);
        log_debug("Tone A : %d %d", g_sound.toneA, rc);
        break;
    case AY_ACOARSE:
        rc = (BYTE)((g_sound.toneA & 0x0F00) >> 8);
        log_debug("Tone A : %d %d", g_sound.toneA, rc);
        break;
    case AY_BFINE:
        rc = (BYTE)(g_sound.toneB & 0x00FF);
        log_debug("Tone B : %d %d", g_sound.toneB, rc);
        break;
    case AY_BCOARSE:
        rc = (BYTE)((g_sound.toneB & 0x0F00) >> 8);
        log_debug("Tone B : %d %d", g_sound.toneB, rc);
        break;
    case AY_CFINE:
        rc = (BYTE)(g_sound.toneC & 0x00FF);
        log_debug("Tone C : %d %d", g_sound.toneC, rc);
        break;
    case AY_CCOARSE:
        rc = (BYTE)((g_sound.toneC & 0x0F00) >> 8);
        log_debug("Tone C : %d %d", g_sound.toneC, rc);
        break;
    case AY_NOISEPER:
        rc = (BYTE)(g_sound.periodNoise & 0x1F);
        log_debug("Rauschperiode: %d %d", g_sound.periodNoise, rc);
        break;
    case AY_ENABLE:
        rc = (BYTE)g_sound.ayStatus;
        log_debug("Enable Status: %d %d", g_sound.ayStatus, rc);
        break;
    case AY_AVOL:
        rc = (BYTE)(g_sound.volA & 0x1F);
        log_debug("Volume A : %d %d", g_sound.volA, rc);
        break;
    case AY_BVOL:
        rc = (BYTE)(g_sound.volB & 0x1F);
        log_debug("Volume B : %d %d", g_sound.volB, rc);
        break;
    case AY_CVOL:
        rc = (BYTE)(g_sound.volC & 0x1F);
        log_debug("Volume C : %d %d", g_sound.volC, rc);
        break;
    case AY_EFINE:
        rc = (BYTE)(g_sound.periodEnv & 0x00FF);
        log_debug("Periode Hüllkurve : %d %d", g_sound.periodEnv, rc);
        break;
    case AY_ECOARSE:
        rc = (BYTE)((g_sound.periodEnv & 0xFF00) >> 8);
        log_debug("Periode Hüllkurve : %d %d", g_sound.periodEnv, rc);
        break;
    case AY_ESHAPE:
        rc = g_sound.shapeEnv & 0x0F;
        log_debug("Hüllkurve : %d %d", g_sound.shapeEnv, rc);
        break;
    case AY_PORTA:
        rc = 0; // not used on NDR-NKC
        break;
    case AY_PORTB:
        rc = 0; // not used on NDR-NKC
        break;
    default:
        break;
    }
    return rc;
}

void sound_p41_out(BYTE data)
{
    log_debug("Writing SOUND-Cmd %2X", data);
    switch (g_sound.ayAddress)
    {
    case AY_AFINE:
        g_sound.toneA = (g_sound.toneA & 0xFF00) + data;
        ayumi_set_tone(&g_sound.ay, 0, g_sound.toneA);
        log_debug("Tone A : %d", g_sound.toneA);
        break;
    case AY_ACOARSE:
        g_sound.toneA = (g_sound.toneA & 0x00FF) + ((data & 0x0F) << 8);
        ayumi_set_tone(&g_sound.ay, 0, g_sound.toneA);
        log_debug("Tone A : %d", g_sound.toneA);
        break;
    case AY_BFINE:
        g_sound.toneB = (g_sound.toneB & 0xFF00) + data;
        ayumi_set_tone(&g_sound.ay, 1, g_sound.toneB);
        log_debug("Tone B : %d", g_sound.toneB);
        break;
    case AY_BCOARSE:
        g_sound.toneB = (g_sound.toneB & 0x00FF) + ((data & 0x0F) << 8);
        ayumi_set_tone(&g_sound.ay, 1, g_sound.toneB);
        log_debug("Tone B : %d", g_sound.toneB);
        break;
    case AY_CFINE:
        g_sound.toneC = (g_sound.toneC & 0xFF00) + data;
        ayumi_set_tone(&g_sound.ay, 2, g_sound.toneC);
        log_debug("Tone C : %d", g_sound.toneC);
        break;
    case AY_CCOARSE:
        g_sound.toneC = (g_sound.toneC & 0x00FF) + ((data & 0x0F) << 8);
        ayumi_set_tone(&g_sound.ay, 2, g_sound.toneC);
        log_debug("Tone C : %d", g_sound.toneC);
        break;
    case AY_NOISEPER:
        g_sound.periodNoise = (data & 0x1F);
        ayumi_set_noise(&g_sound.ay, g_sound.periodNoise);
        log_debug("Rauschperiode: %d", g_sound.periodNoise);
        break;
    case AY_ENABLE:
        // channel A
        g_sound.ayStatus = data;
        int t_off = data & 0x01;
        int n_off = (data >> 3) & 0x01;
        int e_on = (g_sound.volA >> 4) & 0x01;
        log_debug("Channel A is %s Noise %s Envelope %s",
                    (t_off == 1) ? "Off" : "On",
                    (n_off == 1) ? "Off" : "On",
                    (e_on == 1) ? "On" : "Off");
        ayumi_set_mixer(&g_sound.ay, 0, t_off, n_off, e_on);
        // channel B
        t_off = (data >> 1) & 0x01;
        n_off = (data >> 4) & 0x01;
        e_on = (g_sound.volB >> 4) & 0x01;
        log_debug("Channel B is %s Noise %s Envelope %s",
                    (t_off == 1) ? "Off" : "On",
                    (n_off == 1) ? "Off" : "On",
                    (e_on == 1) ? "On" : "Off");
        ayumi_set_mixer(&g_sound.ay, 1, t_off, n_off, e_on);
        // channel C
        t_off = (data >> 2) & 0x01;
        n_off = (data >> 5) & 0x01;
        e_on = (g_sound.volC >> 4) & 0x01;
        log_debug("Channel C is %s Noise %s Envelope %s",
                    (t_off == 1) ? "Off" : "On",
                    (n_off == 1) ? "Off" : "On",
                    (e_on == 1) ? "On" : "Off");
        ayumi_set_mixer(&g_sound.ay, 2, t_off, n_off, e_on);
        break;
    case AY_AVOL:
        g_sound.volA = (BYTE)(data & 0x1F);
        ayumi_set_volume(&g_sound.ay, 0, (g_sound.volA & 0x0F));
        log_debug("Volume A : %d", g_sound.volA);
        t_off = g_sound.ayStatus & 0x01;
        n_off = (g_sound.ayStatus >> 3) & 0x01;
        e_on = (g_sound.volA >> 4) & 0x01;
        log_debug("Channel A is %s  Noise %s Envelope %s", 
                    (t_off == 1) ? "Off" : "On", 
                    (n_off == 1) ? "Off" : "On",
                    (e_on == 1) ? "On" : "Off");
        log_debug("Mixer A:  T_Off:%d, N_Off: %d, e_On: %d", t_off, n_off, e_on);
        ayumi_set_mixer(&g_sound.ay, 0, t_off, n_off, e_on);
        break;
    case AY_BVOL:
        g_sound.volB = (BYTE)(data & 0x1F);
        ayumi_set_volume(&g_sound.ay, 1, (g_sound.volB & 0x0F));
        log_debug("Volume B : %d", g_sound.volB);
        t_off = (g_sound.ayStatus >> 1) & 0x01;
        n_off = (g_sound.ayStatus >> 4) & 0x01;
        e_on = (g_sound.volB >> 4) & 0x01;
        log_debug("Channel B is %s Noise %s Envelope %s", 
                    (t_off == 1) ? "Off" : "On",
                    (n_off == 1) ? "Off" : "On",
                    (e_on == 1) ? "On" : "Off");
        log_debug("Mixer B:  T_Off:%d, N_Off: %d, e_On: %d", t_off, n_off, e_on);
        ayumi_set_mixer(&g_sound.ay, 1, t_off, n_off, e_on);
        break;
    case AY_CVOL:
        g_sound.volC = (BYTE)(data & 0x1F);
        ayumi_set_volume(&g_sound.ay, 2, (g_sound.volC & 0x0F));
        log_debug("Volume C : %d", g_sound.volC);
        t_off = (g_sound.ayStatus >> 2) & 0x01;
        n_off = (g_sound.ayStatus >> 5) & 0x01;
        e_on = (g_sound.volC >> 4) & 0x01;
        log_debug("Channel C is %s Noise %s Envelope %s",
                    (t_off == 1) ? "Off" : "On",
                    (n_off == 1) ? "Off" : "On",
                    (e_on == 1) ? "On" : "Off");
        log_debug("Mixer C:  T_Off:%d, N_Off: %d, e_On: %d", t_off, n_off, e_on);
        ayumi_set_mixer(&g_sound.ay, 2, t_off, n_off, e_on);
        break;
    case AY_EFINE:
        g_sound.periodEnv = (g_sound.periodEnv & 0xFF00) + data;
        ayumi_set_envelope(&g_sound.ay, g_sound.periodEnv);
        log_debug("Hüllkurve Periode: %d", g_sound.periodEnv);
        break;
    case AY_ECOARSE:
        g_sound.periodEnv = (g_sound.periodEnv & 0x00FF) + (data << 8);
        ayumi_set_envelope(&g_sound.ay, g_sound.periodEnv);
        log_debug("Hüllkurve Periode: %d", g_sound.periodEnv);
        break;
    case AY_ESHAPE:
        g_sound.shapeEnv = data & 0x0F;
        ayumi_set_envelope_shape(&g_sound.ay, g_sound.shapeEnv);
        log_debug("Hüllenkurve Form: %d", g_sound.shapeEnv);
        break;
    case AY_PORTA:
        // ignore, not used on ndr-nkc case
        break;
    case AY_PORTB:
        // ignore, not used on ndr-nkc case
        break;

    default:
        break;
    }
}

void ayumi_render(float *sample_data, int length)
{
    float volume = 0.3; // Adjust value
    int frame = 0;
    float out;
    while (frame < FRAME_COUNT)
    {
        ayumi_process(&g_sound.ay);
        out = (float)(g_sound.ay.left * volume);
        if (out > 1.0)
        {
            out = 1.0;
        }
        if (out < -1.0)
        {
            out = 1.0;
        }
        sample_data[frame] = out;
        frame++;
    }
}

void audio_callback(void *userData, Uint8 *stream, int length)
{
    // fill buffer with some sound data
    // We just fill 1000 samles
    ayumi_render(g_sound.sample_data, length / sizeof(float));
    Uint8 *data = (Uint8 *)g_sound.sample_data;

    // put them onto the sound output buffer
    for (int i = 0; i < length; i++)
    {
        stream[i] = data[i];
    }
}

void sound_reset(const char *soundDriver)
{
    if(g_sound.audioDev != 0) {
        SDL_CloseAudioDevice(g_sound.audioDev);         // Close open sound device
        g_sound.audioDev = 0;
        nkc_arr_free(g_sound.devices);                  // Free list of available sound devices
        g_sound.devices = NULL;
    }
    g_sound.ayStatus = 0xFF;

    // Initialize sound sample buffer.
    // If buffer was already initialized (warm-reset), we free it befor
    if (g_sound.sample_data != NULL)
    {
        free(g_sound.sample_data);
        g_sound.sample_data = NULL;
    }
    g_sound.sample_data = (float *) malloc(FRAME_COUNT * sizeof(float));
    memset(g_sound.sample_data, 0, sizeof(float) * FRAME_COUNT);
    if (g_sound.sample_data == NULL)
    {
        log_error("Memory allocation error");
        exit(1);
    }

    // Configure Ayumi library to use AY-3-8912 at 44.100 kHz
    ayumi_configure(&g_sound.ay, 0, 2000000, 44100); // Use AY-3-8912
    ayumi_set_pan(&g_sound.ay, 0, 0., 0);
    ayumi_set_pan(&g_sound.ay, 1, 0., 0);
    ayumi_set_pan(&g_sound.ay, 2, 0., 0);

    SDL_AudioSpec format, obtained;

    /* Format: 32 Bit float, mono, 44,1 KHz */
    format.freq = 44100;
    format.format = AUDIO_F32;
    format.channels = 1;
    format.samples = FRAME_COUNT;
    format.callback = audio_callback;
    format.userdata = NULL;

    /* Open audio device and start playingsound
       (will start calling the callback but be mute as the registers have not been loaded)
     */

    const int numAudioDevices = SDL_GetNumAudioDevices(0);
    if (numAudioDevices == 0)
    {
        log_info("No Audio-Devices found");
    }

    int selectedDevice = -1;
    if(g_sound.devices != NULL)
         nkc_arr_free(g_sound.devices);
    g_sound.devices =  nkc_arr_new();
    const char *nameAudioDevice;
    if (numAudioDevices >= 1)
    {
        for( int i = 0; i < numAudioDevices; i++ )
        {
            nameAudioDevice = SDL_GetAudioDeviceName(i, 0);
            if( soundDriver != NULL)
            {
                if( strncmp(nameAudioDevice,soundDriver,strlen(soundDriver)) == 0)
                    selectedDevice = i;
            }
            nkc_arr_appendString(g_sound.devices,nameAudioDevice);
        }
        if( selectedDevice == -1 && soundDriver != NULL)
        {
            log_warn("No device with name %s found", soundDriver);
            return;
        }

        nameAudioDevice =  SDL_GetAudioDeviceName(selectedDevice, 0),
        log_info("Using audio devices %s", nameAudioDevice);
    }
    if ((g_sound.audioDev = SDL_OpenAudioDevice(nameAudioDevice, 0, &format, &obtained, 0)) == 0)
    {
        log_error("Audio-device could not be opened: %s", SDL_GetError());
        exit(1);
    }

    SDL_PauseAudioDevice(g_sound.audioDev, 0);

}
