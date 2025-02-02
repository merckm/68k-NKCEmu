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

#ifndef HEADER__SOUND
#define HEADER__SOUND
#include <SDL_audio.h>
#include "ayumi/ayumi.h"
#include "nkc.h"
#include "util.h"

#define FRAME_COUNT 1024

typedef struct {
	BYTE_68K ayAddress;
	BYTE_68K ayStatus;
	WORD_68K toneA;
	WORD_68K toneB;
	WORD_68K toneC;
	BYTE_68K volA;
	BYTE_68K volB;
	BYTE_68K volC;
	WORD_68K periodEnv;
	WORD_68K shapeEnv;
	BYTE_68K periodNoise;
	BYTE_68K dataPortA;
	BYTE_68K dataPortB;
	SDL_AudioDeviceID audioDev;
	nkc_array* devices;
	struct ayumi ay;
	float *sample_data;
} sound;

#ifdef __cplusplus
extern "C"
{
#endif

	typedef unsigned short WORD_68K; /* 16 bit unsigned */
	typedef unsigned char BYTE_68K;	 /* 8 bit unsigned */

	BYTE_68K sound_p40_in();
	void sound_p40_out(BYTE_68K data);
	BYTE_68K sound_p41_in();
	void sound_p41_out(BYTE_68K data);
	void sound_reset(const char * soundDriver);

#ifdef __cplusplus
}
#endif

#endif /* HEADER__SOUND */