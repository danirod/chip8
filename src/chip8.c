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

#include <cpu.h>

#include "sdl.h"

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>

/* Flag set by '--hex' */
static int use_hexloader;

/* getopt parameter structure. */
static struct option long_options[] = {
    { "hex", no_argument, &use_hexloader, 1},
    { 0, 0, 0, 0 }
};

/**
 * Print usage. In case you use bad arguments, this will be printed.
 * @param binname how is the program named, usually argv[0].
 */
static void
usage(const char* binname)
{
    printf("Usage: %s [--hex] FILE\n", binname);
    printf("  --hex: if set, will load ROM in hexadecimal mode.\n");
}

char
hex_to_bin(char hex)
{
    if (hex >= '0' && hex <= '9')
        return hex - '0';
    hex &= 0xDF;
    if (hex >= 'A' && hex <= 'F')
        return 10 + (hex - 'A');
    return -1;
}

/**
 * Load a hex file.
 *
 * @param file file path.
 * @param machine data structure to load the HEX into.
 */
static int
load_hex(const char* file, struct machine_t* machine)
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

    // Create a temporal buffer where to store the data.
    char* hexfile = malloc(length);
    if (hexfile == NULL) {
        return 1;
    }

    fread(hexfile, length, 1, fp);
    fclose(fp);

    int mempos = 0x200;

    if (length & 0x01) length--;
    for (int i = 0; i < length; i += 2)
    {
        char hi = hexfile[i];
        char lo = hexfile[i + 1];

        char hi_b = hex_to_bin(hi);
        char lo_b = hex_to_bin(lo);
        if (hi_b == -1 || lo_b == -1) {
            free(hexfile);
            return 1;
        }

        machine->mem[mempos++] = hi_b << 4 | lo_b;
        if (mempos > 0xFFF)
            break;
    }

    free(hexfile);
    return 0;
}

/**
 * Load a ROM into a machine. This function will open a file and load its
 * contents into the memory from the provided machine data structure.
 * In compliance with the specification, ROM data will start at 0x200.
 *
 * @param file file path.
 * @param machine machine data structure to load the ROM into.
 */
static int
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
    SDL_AudioDeviceID dev;
    SDL_AudioSpec* spec;
    struct context_t context;
    struct machine_t mac;
    int must_quit = 0;
    int last_ticks = 0;
    int cycles = 0;

    /* Parse parameters */
    int indexptr, c;
    while ((c = getopt_long(argc, argv, "", long_options, &indexptr)) != -1)
    {
        switch (c)
        {
            case 0:
                // Well, yup.
                break;
            default:
                usage(argv[0]);
                exit(1);
        }
    }

    /*
     * We still may have arguments. getopt.h declares a global variable named
     * optind which contains the index of the next parameter after flags were
     * processed.
     *
     * Which is cool because we need to know the file name to open.
     */
    if (optind >= argc) {
        usage(argv[0]);
        exit(1);
    }

    char* file = argv[optind];

    // Init emulator
    srand(time(NULL));
    init_machine(&mac);
    if (use_hexloader == 0) {
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

    mac.poller = &is_key_down;

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
