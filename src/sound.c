/*
 * chip8 is a CHIP-8 emulator done in C
 * Copyright (C) 2015 Dani Rodríguez <danirod@outlook.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sound.h"
#include <stdlib.h>
#include <math.h>

static float tone_pos = 0;
static float tone_inc = 2 * 3.14159 * 1000 / 44100;

struct audiodata_t
{
    float tone_pos;
    float tone_inc;
};

static void
feed(void* udata, Uint8* stream, int len)
{
    struct audiodata_t* audio = (struct audiodata_t *) udata;
    for (int i = 0; i < len; i++) {
        stream[i] = sinf(audio->tone_pos) + 127;
        audio->tone_pos += audio->tone_inc;
    }
}

SDL_AudioSpec*
init_audiospec(void)
{
    struct audiodata_t* audio = malloc(sizeof(struct audiodata_t));
    audio->tone_pos = 0;
    audio->tone_inc = 2 * 3.14159 * 1000 / 44100;

    SDL_AudioSpec* spec = (SDL_AudioSpec *) malloc(sizeof(SDL_AudioSpec));
    spec->freq = 44100;
    spec->format = AUDIO_U8;
    spec->channels = 1;
    spec->samples = 4096;
    spec->callback = *feed;
    spec->userdata = audio;
    return spec;
}

void
dispose_audiospec(SDL_AudioSpec* spec)
{
    free(spec->userdata);
    free(spec);
}


