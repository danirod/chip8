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

#include <SDL2/SDL.h>
#include "cpu.h"
#include "sdl.h"
#include "display.h"

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
                                       SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
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

    context->surface = SDL_CreateRGBSurface(0, 64, 32, 32,
                                            0x00FF0000, 0x0000FF00,
                                            0x000000FF, 0xFF000000);
    if (context->surface == NULL) {
        goto exception_surface;
    }

    return 0;

    // Exception Treatment:
exception_surface:
    SDL_DestroyTexture(context->texture);
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
    // Seems that DestroyTexture is already freeing context->surface?
    // I don't know, I haven't reviewed SDL2 code, but it crashes
    // if I uncomment this line so I guess so.
    // SDL_FreeSurface(context->surface);

    SDL_DestroyTexture(context->texture);
    SDL_DestroyRenderer(context->renderer);
    SDL_DestroyWindow(context->window);
}

void
render(struct context_t* context, struct machine_t* machine)
{
    // Updates SDL texture using what's on CPU screen.
    SDL_LockTexture(context->texture, NULL,
                    &context->surface->pixels, &context->surface->pitch);
    expand_screen(machine->screen, (Uint32 *) context->surface->pixels);
    SDL_UnlockTexture(context->texture);

    // Render the texture on the renderer.
    SDL_RenderClear(context->renderer);
    SDL_RenderCopy(context->renderer, context->texture, NULL, NULL);
    SDL_RenderPresent(context->renderer);
}
