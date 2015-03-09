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

#include "keyboard.h" 
#include <SDL2/SDL_keyboard.h>

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
