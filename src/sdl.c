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


#include "sdl.h"

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


int
init_context(struct context_t* context)
{
    /*
     * This code simulates exception treatment in C using `goto`.
     * This is actually one of the clever situations where I think that
     * `goto` comes handy and where I'm not against it. I'm not an
     * enlightened supporter for `goto` either but I'm fed up with those
     * jerks that just overly repeat "goto is bad" because somebody told
     * them some time ago without even considering it. Of course I wouldn't
     * use `goto` where any other structure goes better, but if you are
     * very conscious on what you do and it makes a more concise syntax,
     * why not. Of course, if you are on your first year of programming
     * you should avoid `goto` until you are strong on structure programming.
     *
     * In this context, I need to call a set of SDL functions that are
     * chained: eg, the return of one function is the parameter of the next.
     * And I need to rollback those functions if any of them cracks.
     * My Exception Treatment Table at the bottom of this function rollbacks
     * everything and it has labels before every rollback function. If any
     * SDL function fails it just has to jump to the appropiate label and
     * start rollbacking from that entrypoint.
     */

    context->window = SDL_CreateWindow("CHIP-8 Emulator",
                                       SDL_WINDOWPOS_CENTERED,
                                       SDL_WINDOWPOS_CENTERED,
                                       640,
                                       320,
                                       SDL_WINDOW_SHOWN);
    if (context->window == NULL) {
        goto exception_window;
    }

    context->renderer = SDL_CreateRenderer(context->window,
                                           -1, SDL_RENDERER_ACCELERATED);
    if (context->renderer == NULL) {
        goto exception_renderer;
    }

    context->texture = SDL_CreateTexture(context->renderer,
                                         SDL_PIXELFORMAT_RGBA8888,
                                         SDL_TEXTUREACCESS_STREAMING, 64, 32);
    if (context->texture == NULL) {
        goto exception_texture;
    }

    return 0;

    // Exception Treatment:
exception_texture:
    SDL_DestroyRenderer(context->renderer);
exception_renderer:
    SDL_DestroyWindow(context->window);
exception_window: // Nothing left to rollback; nothing was created
    return 1;
}

void
destroy_context(struct context_t* context)
{
    SDL_DestroyTexture(context->texture);
    SDL_DestroyRenderer(context->renderer);
    SDL_DestroyWindow(context->window);
}

void
render(struct context_t* context, struct machine_t* machine)
{
    void*   pixels;
    int     pitch;

    // Updates SDL texture using what's on CPU screen.
    SDL_LockTexture(context->texture, NULL,
                    &pixels, &pitch);
    expand_screen(machine->screen, (Uint32 *) pixels);
    SDL_UnlockTexture(context->texture);

    // Render the texture on the renderer.
    SDL_RenderClear(context->renderer);
    SDL_RenderCopy(context->renderer, context->texture, NULL, NULL);
    SDL_RenderPresent(context->renderer);
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
expand_screen(char* from, Uint32* to)
{
    for (int i = 0; i < 2048; i++)
        to[i] = ( from[i]) ? -1 : 0;
}

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
