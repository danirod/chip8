#include <SDL2/SDL.h>
#include "cpu.h"
#include "sdl.h"
#include "display.h"

int
init_context(struct context_t* context)
{
    context->window = SDL_CreateWindow("CHIP-8 Emulator",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              640, 320, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if (context->window == NULL)
        goto ERROR_WINDOW;

    context->renderer = SDL_CreateRenderer(context->window,
                                           -1, SDL_RENDERER_ACCELERATED);
    if (context->renderer == NULL)
        goto ERROR_RENDERER;

    context->texture = SDL_CreateTexture(context->renderer,
                                         SDL_PIXELFORMAT_RGBA8888,
                                         SDL_TEXTUREACCESS_STREAMING, 64, 32);
    if (context->texture == NULL)
        goto ERROR_TEXTURE;

    context->surface = SDL_CreateRGBSurface(0, 64, 32, 32,
                                            0x00FF0000, 0x0000FF00,
                                            0x000000FF, 0xFF000000);
    if (context->surface == NULL)
        goto ERROR_SURFACE;

    return 0;
ERROR_SURFACE:
    SDL_DestroyTexture(context->texture);
ERROR_TEXTURE:
    SDL_DestroyRenderer(context->renderer);
ERROR_RENDERER:
    SDL_DestroyWindow(context->window);
ERROR_WINDOW:
    return 1;
}

void
destroy_context(struct context_t* context)
{
    SDL_FreeSurface(context->surface);
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
