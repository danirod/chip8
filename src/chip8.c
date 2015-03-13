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

#include "cpu.h"
#include "keyboard.h"
#include "sdl.h"
#include "sound.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>

/**
 * Load a ROM into a machine. This function will open a file and load its
 * contents into the memory from the provided machine data structure.
 * In compliance with the specification, ROM data will start at 0x200.
 *
 * @param machine machine data structure to load the ROM into.
 */
int
load_rom(const char* file, struct machine_t* machine)
{
    FILE* fp = fopen(file, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Cannot open ROM file.\n");
        return 1;
    }

    // Use the fseek/ftell/fseek trick to retrieve file size.
    fseek(fp, 0, SEEK_END);
    int length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Check the length of the rom. Must be as much 3584 bytes long, which
    // is 4096 - 512. Since first 512 bytes of memory are reserved, program
    // code can only allocate up to 3584 bytes. Must check for bounds in
    // order to avoid buffer overflows.
    if (length > 3584) {
        fprintf(stderr, "ROM too large.\n");
        return 1;
    }

    // Everything is OK, read the ROM.
    fread(machine->mem + 0x200, length, 1, fp);
    fclose(fp);
    return 0;
}

int
main(int argc, char** argv)
{
    SDL_AudioSpec* spec;
    struct context_t context;
    struct machine_t mac;
    int must_quit = 0;
    int last_ticks = 0;
    int cycles = 0;

    // Read ROM file to load.
    if (argc == 1) {
        fprintf(stderr, "Usage: %s ROMFILE\n", argv[0]);
        return 1;
    }

    // Init emulator
    srand(time(NULL));
    init_machine(&mac);
    if (load_rom(argv[1], &mac))
        return 1;

    // Init SDL engine
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "SDL Initialization Error: %s\n", SDL_GetError());
        return 1;
    }
    spec = init_audiospec();
    if (SDL_OpenAudio(spec, NULL) < 0) {
        return 1;
    }
    if (init_context(&context) != 0) {
        fprintf(stderr, "SDL Context Error: %s\n", SDL_GetError());
        return 1;
    }

    // Main loop.
    while (!must_quit) {
        // Check for events.
        while (SDL_PollEvent(&context.event)) {
            if (context.event.type == SDL_QUIT) {
              must_quit = 1;
            }
        }

        // Execute a machine instruction.
        if (SDL_GetTicks() - cycles > 1) {
            /*
             * If we are waiting to the user for pressing a key, we must not
             * execute a machine instrucion, as the machine is halted.
             */
            if (mac.wait_key == -1) {
                step_machine(&mac);
            } else {
                for (int key = 0; key <= 0xF; key++) {
                    if (is_key_down(key)) {
                        mac.v[(int) mac.wait_key] = key;
                        mac.wait_key = -1;
                        break;
                    }
                }
            }
            cycles = SDL_GetTicks();
        }

        // Render the display and update timers each frame.
        if (SDL_GetTicks() - last_ticks > (1000 / 60)) {
            if (mac.dt) mac.dt--;
            if (mac.st) {
                if (--mac.st == 0)
                    SDL_PauseAudio(1);
            }
            render(&context, &mac);
            last_ticks = SDL_GetTicks();
        }
    }

    // Dispose SDL engine.
    destroy_context(&context);
    SDL_CloseAudio();
    dispose_audiospec(spec);
    SDL_Quit();
    return 0;
}
