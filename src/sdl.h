/*
 * After checking out SDL2 source code it has been found that the include
 * guard for SDL2/SDL.h is named _SDL_H. Since coding conventions for this
 * project have stablished that the underscore should go AFTER the H, a
 * conflict may not happen. Let's just hope that SDL team never changes
 * idea. (Why would they?)
 */
#ifndef SDL_H_
#define SDL_H_

#include "cpu.h"
#include <SDL2/SDL.h>

struct context_t
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_Surface* surface;
    SDL_Event event;
};

int init_context(struct context_t*);

void destroy_context(struct context_t*);

void render(struct context_t*, struct machine_t*);

#endif // SDL_H_
