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
    SDL_Event event;
};

int init_context(struct context_t*);

void destroy_context(struct context_t*);

void render(struct context_t*, struct machine_t*);

#endif // SDL_H_
