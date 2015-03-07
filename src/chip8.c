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
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <SDL2/SDL.h>

#define MEMSIZ 4096 // How much memory can handle the CHIP-8

char hexcodes[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

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
 * Main data structure for holding information and state about processor.
 * Memory, stack, and register set is all defined here.
 */
struct machine_t
{
    uint8_t mem[MEMSIZ];        // Memory is allocated as a buffer
    uint16_t pc;                // Program Counter
    
    uint16_t stack[16];         // Stack can hold 16 16-bit values
    uint16_t sp;                // Stack pointer
    
    uint8_t v[16];              // 16 general purpose registers
    uint16_t i;                 // Special I register
    uint8_t dt, st;             // Timers

    char screen[2048];          // Pantalla
    char wait_key;              // Tecla que estoy esperando
};

static int
is_key_down(char key)
{
    const Uint8* sdl_keys = SDL_GetKeyboardState(NULL);
    Uint8 real_key = keys[(int) key];
    return sdl_keys[real_key];
}

static void
expansion(char* from, Uint32* to)
{
    for (int i = 0; i < 2048; i++)
        to[i] = ( from[i]) ? -1 : 0;
}

/**
 * Initializes to cero a machine data structure. This function should be
 * called when the program is starting up to make sure that the machine
 * data structure is getting initialized. It also can be called everytime
 * the user wants the machine to be reinitialized, such as a reboot.
 * 
 * @param machine machine data structure that wants to be initialized.
 */
void
init_machine(struct machine_t* machine)
{
    memset(machine, 0x00, sizeof(struct machine_t));
    memcpy(machine->mem + 0x50, hexcodes, 80);
    machine->pc = 0x200;
    machine->wait_key = -1;
}

/**
 * Load a ROM into a machine. This function will open a file and load its
 * contents into the memory from the provided machine data structure.
 * In compliance with the specification, ROM data will start at 0x200.
 *
 * @param machine machine data structure to load the ROM into.
 */
void
load_rom(struct machine_t* machine)
{
    FILE* fp = fopen("PONG", "r");
    // TODO: Should I use exit or return a flag value? Discuss! 
    if (fp == NULL) {
        fprintf(stderr, "Cannot open ROM file.\n");
        exit(1);
    }
    
    // Use the fseek/ftell/fseek trick to retrieve file size.
    fseek(fp, 0, SEEK_END);
    int length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    // Then oh, please, please, please read the file.
    fread(machine->mem + 0x200, length, 1, fp);
    fclose(fp);
}

/**
 * Step the machine. This method will fetch an instruction from memory
 * and execute it. After invoking this method, the state of the provided
 * machine is modified according to the executed instruction.
 * 
 * @param cpu reference pointer to the machine to step
 */
void
step_machine(struct machine_t* cpu)
{
    // Read next opcode from memory.
    uint16_t opcode = (cpu->mem[cpu->pc] << 8) | cpu->mem[cpu->pc + 1];
    cpu->pc = (cpu->pc + 2) & 0xFFF;

    // Extract bit nibbles from the opcode
    uint16_t nnn = opcode & 0x0FFF;
    uint8_t kk = opcode & 0xFF;
    uint8_t n = opcode & 0xF;
    uint8_t x = (opcode >> 8) & 0xF;
    uint8_t y = (opcode >> 4) & 0xF;
    uint8_t p = (opcode >> 12);

    // infernal switch case! (sorry for all the heart attacks here u_u)
    switch (p) {
        case 0:
            if (opcode == 0x00e0) {
                // CLS
                memset(cpu->screen, 0, 2048);
            } else if (opcode == 0x00ee) {
                if (cpu->sp > 0)
                    cpu->pc = cpu->stack[--cpu->sp];
            }
            break;
        case 1:
            // jp nnn: set program counter to nnn
            cpu->pc = nnn;
            break;
        case 2:
            // call nnn: stack[sp++] = pc, pc = nnn
            if (cpu->sp < 16)
                cpu->stack[cpu->sp++] = cpu->pc;
            cpu->pc = nnn;
            break;
        case 3:
            // se x, kk: if v[x] == kk -> pc += 2
            if (cpu->v[x] == kk)
                cpu->pc = (cpu->pc + 2) & 0xfff;
            break;
        case 4:
            // sne x, kk: if v[x] != kk -> pc += 2
            if (cpu->v[x] != kk)
                cpu->pc = (cpu->pc + 2) & 0xfff;
            break;
        case 5:
            // se x, y: if v[x] == v[y] -> pc += 2
            if (cpu->v[x] == cpu->v[y])
                cpu->pc = (cpu->pc + 2) & 0xfff;
            break;
        case 6:
            // ld x, kk: v[x] = kk
            cpu->v[x] = kk;
            break;
        case 7:
            // add x, kk: v[x] = (v[x] + kk) & 0xff
            cpu->v[x] = (cpu->v[x] + kk) & 0xff;
            break;
        case 8:
            switch (n) {
                case 0:
                    // ld x, y: v[x] = v[y]
                    cpu->v[x] = cpu->v[y];
                    break;
                case 1:
                    // or x, y: v[x] = v[x] | v[y];
                    cpu->v[x] |= cpu->v[y];
                    break;
                case 2:
                    // and x, y: v[x] = v[x] & v[y]
                    cpu->v[x] &= cpu->v[y];
                    break;
                case 3:
                    // xor x, y: v[x] = v[x] ^ v[y]
                    cpu->v[x] ^= cpu->v[y];
                    break;
                case 4:
                    // add x, y: v[x] += v[y]
                    cpu->v[0xf] = (cpu->v[x] > cpu->v[x] + cpu->v[y]);
                    cpu->v[x] += cpu->v[y];
                    break;
                case 5:
                    // SUB x, y: V[x] -= V[y]
                    cpu->v[0xF] = (cpu->v[x] > cpu->v[y]);
                    cpu->v[x] -= cpu->v[y];
                    break;
                case 6:
                    // SHR x : V[x] = V[x] >> 1
                    cpu->v[0xF] = (cpu->v[x] & 1);
                    cpu->v[x] >>= 1;
                    break;
                case 7:
                    // SUBN x, y: V[x] = V[y] - V[x]
                    cpu->v[0xF] = (cpu->v[y] > cpu->v[x]);
                    cpu->v[x] = cpu->v[y] - cpu->v[x];
                    break;
                case 0xE:
                    // SHL x : V[x] = V[x] << 1
                    cpu->v[0xF] = ((cpu->v[x] & 0x80) != 0);
                    cpu->v[x] <<= 1;
                    break;
            }
            break;
        case 9:
            // SNE x, y: V[x] != V[y] -> pc += 2;
            if (cpu->v[x] != cpu->v[y])
                cpu->pc = (cpu->pc + 2) & 0xFFF;
            break;
        case 0xA:
            // LD I, x : I = nnn
            cpu->i = nnn;
            break;
        case 0xB:
            // JP V0, nnn: pc = V[0] + nnn
            cpu->pc = (cpu->v[0] + nnn) & 0xFFF;
            break;
        case 0xC:
            // RND x, kk: V[x] = random() % kk
            cpu->v[x] = rand() & kk;
            break;
        case 0xD:
            /*
             * DRW x, y, n:
             * Dibuja un sprite en el pixel v[x], v[y].
             * El número de filas a dibujar se dice con n.
             * El sprite se saca de la dirección de memoria [I].
             */
            cpu->v[15] = 0;
            for (int j = 0; j < n; j++) {
                uint8_t sprite = cpu->mem[cpu->i + j];
                for (int i = 0; i < 8; i++) {
                    int px = (cpu->v[x] + i) & 63;
                    int py = (cpu->v[y] + j) & 31;
                    int pos = 64 * py + px;
                    int pixel = (sprite & (1 << (7-i))) != 0;
                    
                    cpu->v[15] |= (cpu->screen[pos] & pixel);
                    cpu->screen[pos] ^= pixel;
                }
            }
            break;
        case 0xE:
            if (kk == 0x9E) {
                // SKP x: if key V[x] is down, skip next instruction
                if (is_key_down(cpu->v[x]))
                    cpu->pc = (cpu->pc + 2) & 0xFFF;
            } else if (kk == 0xA1) {
                // SKNP x
                if (!is_key_down(cpu->v[x]))
                    cpu->pc = (cpu->pc + 2) & 0xFFF;
            }
            break;
        case 0xF:
            switch (kk) {
                case 0x07:
                    // LD V[x], DT: V[x] = DT
                    cpu->v[x] = cpu->dt;
                    break;
                case 0x0A:
                    // LD X, K
                    cpu->wait_key = x;
                    break;
                case 0x15:
                    // LD DT, V[x] -> DT = V[x]
                    cpu->dt = cpu->v[x];
                    break;
                case 0x18:
                    // LD ST, V[x] -> ST = V[x]
                    cpu->st = cpu->v[x];
                    break;
                case 0x1E:
                    // ADD I, V[x] -> I += V[x]
                    cpu->i += cpu->v[x];
                    break;
                case 0x29:
                    // LD F, V[x] -> I = [posicion de memoria del número V[x]]
                    cpu->i = 0x50 + (cpu->v[x] & 0xF) * 5;
                    break;
                case 0x33:
                    // LD B, V[x] = loads BCD number in memory
                    cpu->mem[cpu->i + 2] = cpu->v[x] % 10;
                    cpu->mem[cpu->i + 1] = (cpu->v[x] / 10) % 10;
                    cpu->mem[cpu->i] = (cpu->v[x] / 100); 
                    break;
                case 0x55:
                    // LD [I], X -> guarda en I
                    for (int reg = 0; reg <= x; reg++)
                        cpu->mem[cpu->i + reg] = cpu->v[reg];
                    break;
                case 0x65:
                    // LD X, [I] -> lee de I
                    for (int reg = 0; reg <= x; reg++)
                        cpu->v[reg] = cpu->mem[cpu->i + reg];
                    break;
            }
            break;
    }
}

int
main(int argc, const char * argv[])
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_Surface* surface;
    SDL_Event event;
    struct machine_t mac;
    int mustQuit = 0;
    int last_ticks = 0;

    // Init emulator
    init_machine(&mac);
    load_rom(&mac);
    srand(time(NULL));

    // Init SDL engine
    SDL_Init(SDL_INIT_EVERYTHING);
    window = SDL_CreateWindow("CHIP-8 Emulator",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              640, 320, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                SDL_TEXTUREACCESS_STREAMING, 64, 32);
    
    // Write some dummy white background
    surface = SDL_CreateRGBSurface(0, 64, 32, 32,
                                       0x00FF0000,
                                       0x0000FF00,
                                       0x000000FF,
                                       0xFF000000);
    SDL_LockTexture(texture, NULL, &surface->pixels, &surface->pitch);
    expansion(mac.screen, (Uint32 *) surface->pixels);
    SDL_UnlockTexture(texture);
    
    int cycles = 0;

    while (!mustQuit) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    mustQuit = 1;
                    break;
            }
        }
        
        if (SDL_GetTicks() - cycles > 1) {
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

        if (SDL_GetTicks() - last_ticks > (1000 / 60)) {
            if (mac.dt) mac.dt--;
            if (mac.st) mac.st--;

            SDL_LockTexture(texture, NULL, &surface->pixels, &surface->pitch);
            expansion(mac.screen, (Uint32 *) surface->pixels);
            SDL_UnlockTexture(texture);

            // SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, NULL, NULL);
            SDL_RenderPresent(renderer);
            last_ticks = SDL_GetTicks();
        }
    }
    
    // Dispose SDL engine.
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
