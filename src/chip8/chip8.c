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

#include <lib8/cpu.h>
#include "libsdl.h"
#include <config.h>

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Flag set by '--hex' */
static int use_hexloader;

/* Flag set by '--mute' */
static int use_mute;

/* Flag used by '--debug' */
static int use_debug;

/* Opcodes to execute per frame. */
static int speed = 16;

/* getopt parameter structure. */
static struct option long_options[] = {
    { "help", no_argument, 0, 'h' },
    { "version", no_argument, 0, 'v' },
    { "hex", no_argument, &use_hexloader, 1 },
    { "mute", no_argument, &use_mute, 1 },
    { "debug", no_argument, &use_debug, 1 },
    { "speed", required_argument, 0, 's' },
    { 0, 0, 0, 0 }
};

/**
 * Print usage. In case you use bad arguments, this will be printed.
 * @param name how is the program named, usually argv[0].
 */
static void
usage(const char* name)
{
    /* How many characters has Usage: %s? */
    int pad = strnlen(name, 10) + 7; // 7 = "Usage: "

    printf("Usage: %s [-h | --help] [-v | --version]\n", name);
    printf("%*c [--hex] [--mute] <file>\n", pad, ' ');
}

static char
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
        char hi = hex_to_bin(hexfile[i]);
        char lo = hex_to_bin(hexfile[i + 1]);
        if (hi == -1 || lo == -1) {
            free(hexfile);
            return 1;
        }

        machine->mem[mempos++] = hi << 4 | lo;
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

static int
load_data(char* file, struct machine_t* mac)
{
    if (use_hexloader == 0) {
        return load_rom(file, mac);
    } else {
        return load_hex(file, mac);
    }
}

int
main(int argc, char** argv)
{
    struct machine_t mac;

    /* Parse parameters */
    int indexptr, c;
    while ((c = getopt_long(argc, argv, "hs:v", long_options, &indexptr))
           != -1) {
        switch (c) {
            case 'h':
                usage(argv[0]);
                exit(0);
            case 's':
                if (optarg) {
                    speed = atoi(optarg);
                }
                if (speed <= 0) {
                    fprintf(
                        stderr,
                        "Invalid speed value: must be a positive number\n");
                    exit(1);
                }
                break;
            case 'v':
                printf("%s\n", PACKAGE_STRING);
                exit(0);
                break;
            case 0:
                /* A long option is being processed, probably --hex. */
                break;
            default:
                exit(1);
        }
    }

    /*
     * optind should have the index of next parameter in argv. It should be
     * the name of the file to read. Therefore, should complain if file is not
     * given.
     */
    if (optind >= argc) {
        fprintf(stderr, "%1$s: no file given. '%1$s -h' for help.\n", argv[0]);
        exit(1);
    }

    printf("CHIP-8 emulator\n");
    printf("Speed emulation: %d\n", speed);

    /* Initialize SDL Context. */
    if (init_context()) {
        fprintf(stderr, "Error initializing SDL graphical context:\n");
        fprintf(stderr, "%s\n", SDL_GetError());
        return 1;
    }
    if (!try_enable_sound()) {
        fprintf(stderr, "Couldn't enable sound.\n");
        use_mute = 1;
    }

    /* Init emulator. */
    srand(time(NULL));
    if (use_debug) {
        set_debug_mode(1);
    }
    init_machine(&mac);
    mac.keydown = &is_key_down;
    if (!use_mute) {
        mac.speaker = &update_speaker;
    }
    load_data(argv[optind], &mac);

    int last_ticks = SDL_GetTicks();
    int last_delta = 0;
    while (!is_close_requested()) {
        /* Update timers. */
        last_delta = SDL_GetTicks() - last_ticks;
        last_ticks = SDL_GetTicks();

        /* Update computer. */
        update_time(&mac, last_delta);
        for (int i = 0; i < speed; i++) {
            step_machine(&mac);
        }

        /* Render computer. */
        render_display(&mac);

        /*
         * To render at 60 Hz, you must render a frame each 16.6 ms.
         * If it took less than 16.6 ms, you can afford sleep some
         * time. 1 ms will always be slept to keep the CPU cold.
         */
        int render_time = SDL_GetTicks() - last_ticks;
        if (render_time < 16) {
            /* We can sleep :) */
            SDL_Delay(16 - render_time);
        } else {
            /* We should not sleep, but we will do because the CPU will
             * otherwise explode. */
            SDL_Delay(1);
        }
    }

    /* Dispose SDL context. */
    destroy_context();

    return 0;
}
