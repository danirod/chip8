/*
 * chip8 is a CHIP-8 emulator done in C
 * Copyright (C) 2015-2016 Dani Rodríguez <danirod@outlook.com>
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


#include "libsdl.h"

#include <stdlib.h>
#include <math.h>

/**
 * This array maps a SDL scancode to the CHIP-8 key that serves as index
 * for the array. For instance, pressing key keys[5] on your keyboard will
 * make your CHIP-8 understand as if you pressed the [5] key. Remember that
 * CHIP-8 uses a 16-key keyboard with keys labeled 0..F.
 */
char keys[] = {
    SDL_SCANCODE_X, // 0
    SDL_SCANCODE_1, // 1
    SDL_SCANCODE_2, // 2
    SDL_SCANCODE_3, // 3
    SDL_SCANCODE_Q, // 4
    SDL_SCANCODE_W, // 5
    SDL_SCANCODE_E, // 6
    SDL_SCANCODE_A, // 7
    SDL_SCANCODE_S, // 8
    SDL_SCANCODE_D, // 9
    SDL_SCANCODE_Z, // A
    SDL_SCANCODE_C, // B
    SDL_SCANCODE_4, // C
    SDL_SCANCODE_R, // D
    SDL_SCANCODE_F, // E
    SDL_SCANCODE_V  // F
};

/**
 * This is a private structure used for holding information about audio.
 * I need to create the structure becuase the feeding function for audio
 * in SDL only allows one single parameter to be provided via user data.
 * This little trick lets me pass more than one variable.
 */
struct audiodata_t
{
    float tone_pos;
    float tone_inc;
};

static SDL_Window* window = NULL;

static SDL_Renderer* renderer = NULL;

static SDL_Texture* texture = NULL;

static SDL_AudioDeviceID device = 0;

static SDL_AudioSpec* spec = NULL;

/**
 * This is the function that generates the beep noise heard in the emulator.
 * It generates RAW PCM values that are written to the stream. This is fast
 * and has no dependencies on external files.
 */
static void
feed(void* udata, Uint8* stream, int len)
{
    struct audiodata_t* audio = (struct audiodata_t *) udata;
    for (int i = 0; i < len; i++) {
        stream[i] = sinf(audio->tone_pos) + 127;
        audio->tone_pos += audio->tone_inc;
    }
}

/**
  * Generate an audiospec data structure ready to be used by SDL Audio.
  * It would be a nice idea if the sampling frequency and buffer size could
  * be provided as an input instead of being hardcoded, though.
  */
static SDL_AudioSpec*
init_audiospec(void)
{
    /* Initialize user data structure. */
    struct audiodata_t* audio = malloc(sizeof(struct audiodata_t));
    audio->tone_pos = 0;
    audio->tone_inc = 2 * 3.14159 * 1000 / 44100;

    /* Set up the audiospec data structure required by SDL. */
    spec = (SDL_AudioSpec *) malloc(sizeof(SDL_AudioSpec));
    spec->freq = 44100;
    spec->format = AUDIO_U8;
    spec->channels = 1;
    spec->samples = 4096;
    spec->callback = *feed;
    spec->userdata = audio;
    return spec;
}

static void
clean_up()
{
    if (device != 0) {
        SDL_CloseAudioDevice(device);
        device = 0;
    }
    if (spec != NULL) {
        free(spec->userdata);
        free(spec);
        spec = NULL;
    }
    if (texture != NULL) {
        SDL_DestroyTexture(texture);
        texture = NULL;
    }
    if (renderer != NULL) {
        SDL_DestroyRenderer(renderer);
        renderer = NULL;
    }
    if (window != NULL) {
        SDL_DestroyWindow(window);
        window = NULL;
    }
    SDL_Quit();
}

#define TEXTURE_PIXEL(x, y) (128 * (y) + (x))

static void
expand_screen(char* from, Uint32* to, int use_hdpi)
{
    if (use_hdpi) {
        for (int i = 0; i < 8192; i++)
            to[i] = from[i] ? -1 : 0;
    } else {
        int x = 0, y = 0;
        for (int i = 0; i < 2048; i++) {
            Uint32 val = from[i] ? -1 : 0;
            to[TEXTURE_PIXEL(2 * x + 0, 2 * y + 0)] = val;
            to[TEXTURE_PIXEL(2 * x + 1, 2 * y + 0)] = val;
            to[TEXTURE_PIXEL(2 * x + 0, 2 * y + 1)] = val;
            to[TEXTURE_PIXEL(2 * x + 1, 2 * y + 1)] = val;
            if (++x == 64) {
                x = 0;
                y++;
            }
        }
    }
}

int
init_context()
{
    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        return 1;
    }
    window = SDL_CreateWindow("CHIP-8 Emulator",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            640, 320, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        clean_up();
        return 1;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        clean_up();
        return 1;
    }
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
            SDL_TEXTUREACCESS_STREAMING, 128, 64);
    if (texture == NULL) {
        clean_up();
        return 1;
    }
    return 0;
}

int
try_enable_sound()
{
    spec = init_audiospec();
    device = SDL_OpenAudioDevice(NULL, 0, spec,
            NULL, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    return (device != 0);
}

void
destroy_context()
{
    clean_up();
}

int
is_close_requested()
{
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT) {
            return 1;
        }
    }
    return 0;
}

void
render_display(struct machine_t* machine)
{
    void*   pixels;
    int     pitch;

    /* Update SDL Texture with current data in CPU. */
    SDL_LockTexture(texture, NULL, &pixels, &pitch);
    expand_screen(machine->screen, (Uint32 *) pixels, machine->esm);
    SDL_UnlockTexture(texture);

    /* Render the texture. */
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
}

/**
 * Checks if a given key is pressed. This function acceps a CHIP-8 key in
 * range 0-F. It will check using SDL if the PC keyboard mapped to that
 * CHIP-8 key is acutally being pressed or not.
 *
 * @param key CHIP-8 key to be checked.
 * @return 0 if that key is not down; != 0 if that key IS down.
 */
int
is_key_down(char key)
{
    const Uint8* sdl_keys; // SDL key array information
    Uint8 real_key; // Mapped SDL scancode for the given key
    if (key < 0 || key > 15) return 0; // check those bounds.

    sdl_keys = SDL_GetKeyboardState(NULL);
    real_key = keys[(int) key];
    return sdl_keys[real_key];
}

void
update_speaker(int enabled)
{
    if (enabled) {
        SDL_PauseAudioDevice(device, 0);
    } else {
        SDL_PauseAudioDevice(device, 1);
    }
}
