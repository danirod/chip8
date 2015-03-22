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
#include "rom.h"
#include "keyboard.h"
#include "sdl.h"
#include "sound.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>

int
main(int argc, char** argv)
{
    SDL_AudioDeviceID dev;
    SDL_AudioSpec* spec;
    struct context_t context;
    struct machine_t mac;
    int must_quit = 0;
    int last_ticks = 0;
    int cycles = 0;

/*
 * chip8 PONG
 * chip8 -h PONG.HEX
 */

    int load_type = 0; // 0 -> load_rom; 1 -> load_hex
    char* file;

    // Read ROM file to load.
    if (argc == 2) {
        load_type = 0;
        file = argv[1];
    } else if (argc == 3 && memcmp(argv[1], "-h", 2) == 0) {
        load_type = 1;
        file = argv[2];
    } else {
        printf("Usage: %s [-h] ROMFILE\n", argv[0]);
        printf("  -h: if set, will load as hex file.\n");
        return 1;
    }

    // Init emulator
    srand(time(NULL));
    init_machine(&mac);
    if (load_type == 0) {
        if (load_rom(file, &mac)) {
            return 1;
        }
    } else {
        if (load_hex(file, &mac)) {
            return 1;
        }
    }

    // Init SDL engine
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "SDL Initialization Error: %s\n", SDL_GetError());
        return 1;
    }
    spec = init_audiospec();
    dev = SDL_OpenAudioDevice(NULL, 0, spec, NULL,
                              SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if (dev == 0) {
        fprintf(stderr, "SDL Sound Error: %s\n", SDL_GetError());
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
                    SDL_PauseAudioDevice(dev, 1);
                else
                    SDL_PauseAudioDevice(dev, 0);
            }
            render(&context, &mac);
            last_ticks = SDL_GetTicks();
        }
    }

    // Dispose SDL engine.
    destroy_context(&context);
    SDL_CloseAudioDevice(dev);
    dispose_audiospec(spec);
    SDL_Quit();
    return 0;
}
