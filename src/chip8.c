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

/**
 * These are the bitmaps for the sprites that represent numbers.
 * This array should be memcopied to memory address 0x050. LD F, Vx
 * instruction sets I register to the memory address of a provided
 * number.
 */
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

    char screen[2048];          // Screen bitmap
    char wait_key;              // Key the CHIP-8 is idle waiting for.
};

/**
 * Checks if a given key is pressed. This function acceps a CHIP-8 key in
 * range 0-F. It will check using SDL if the PC keyboard mapped to that
 * CHIP-8 key is acutally being pressed or not.
 *
 * @param key CHIP-8 key to be checked.
 * @return 0 if that key is not down; != 0 if that key IS down.
 */
static int
is_key_down(char key)
{
    const Uint8* sdl_keys; // SDL key array information
    Uint8 real_key; // Mapped SDL scancode for the given key
    if (key < 0 || key > 15) return 0; // check those bounds.

    sdl_keys = SDL_GetKeyboardState(NULL);
    real_key = keys[(int) key];
    return sdl_keys[real_key];
}

/**
 * Expand a machine_t bitmap into a SDL 32-bit surface bitmap. This function
 * will work provided some requirements are meet. Both arrays must be 2048
 * positions long. Input array must be a char array where each position is
 * either 0 or 1.
 *
 * Output array will be expanded in a format that is compatible with SDL
 * surfaces. Each 0 on input array will be converted to a 0 on output array,
 * which will result on no pixel being drawn on that position. Each 1 on
 * input array will be converted to a -1 on output array, which being unsigned
 * will result on every single bit of that position being 1 and thus
 * plotting a white pixel.
 *
 * @param from input array where the struct machine_t bitmap is from.
 * @param to output array where the pixels will be blitted.
 */
static void
expand_screen(char* from, Uint32* to)
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
int
load_rom(const char* file, struct machine_t* machine)
{
    FILE* fp = fopen(file, "r");
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
                /*
                 * 00E0: CLS
                 * Clear the screen
                 */
                memset(cpu->screen, 0, 2048);
            } else if (opcode == 0x00ee) {
                /*
                 * 00EE: RET
                 * Return from subroutine.
                 */
                if (cpu->sp > 0)
                    cpu->pc = cpu->stack[--cpu->sp];
            }
            break;

        case 1:
            /*
             * 1NNN: JMP NNN
             * Jump to address location NNN.
             */
            cpu->pc = nnn;
            break;

        case 2:
            /*
             * 2NNN: CALL NNN
             * Call subroutine starting at address location NNN.
             */
            if (cpu->sp < 16)
                cpu->stack[cpu->sp++] = cpu->pc;
            cpu->pc = nnn;
            break;
        case 3:
            /*
             * 3XKK: SE X, KK
             * Skip next instruction if V[X] == KK
             */
            if (cpu->v[x] == kk)
                cpu->pc = (cpu->pc + 2) & 0xfff;
            break;

        case 4:
            /*
             * 4XKK: SNE X, KK
             * SKip next instruction if V[X] != KK
             */
            if (cpu->v[x] != kk)
                cpu->pc = (cpu->pc + 2) & 0xfff;
            break;

        case 5:
            /*
             * 5XY0: SE X, Y
             * Skip next instruction if V[X] == V[Y].
             */
            if (cpu->v[x] == cpu->v[y])
                cpu->pc = (cpu->pc + 2) & 0xfff;
            break;

        case 6:
            /*
             * 6XKK: LD X, KK
             * Set V[x] = KK.
             */
            cpu->v[x] = kk;
            break;

        case 7:
            /*
             * 7XKK: ADD X, KK
             * Add KK to V[X].
             */
            cpu->v[x] = (cpu->v[x] + kk) & 0xff;
            break;

        case 8:
            switch (n) {
                case 0:
                    /*
                     * 8XY0: LD X, Y
                     * Set V[x] = V[y]
                     */
                    cpu->v[x] = cpu->v[y];
                    break;

                case 1:
                    /*
                     * 8XY1: OR X, Y
                     * Set V[x] to V[x] OR V[y].
                     */
                    cpu->v[x] |= cpu->v[y];
                    break;
                    
                case 2:
                    /*
                     * 8XY2: AND X, Y
                     * Set V[x] to V[x] AND V[y].
                     */
                    cpu->v[x] &= cpu->v[y];
                    break;

                case 3:
                    /*
                     * 8XY3: XOR X, Y
                     * Set V[x] to V[x] XOR V[y]
                     */
                    cpu->v[x] ^= cpu->v[y];
                    break;

                case 4:
                    /*
                     * 8XY4: ADD X, Y
                     * Add V[y] to V[x]. V[15] is used as carry flag: if
                     * there is a carry, V[15] must be set to 1, else to 0.
                     */
                    cpu->v[0xf] = (cpu->v[x] > cpu->v[x] + cpu->v[y]);
                    cpu->v[x] += cpu->v[y];
                    break;

                case 5:
                    /*
                     * 8XY5: SUB X, Y
                     * Substract V[y] from V[x]. V[15] is used as borrow flag:
                     * if there is a borrow, V[15] must be set to 0, else 
                     * to 1. Which in practice is easier to check as if
                     * V[x] is greater than V[y].
                     */
                    cpu->v[0xF] = (cpu->v[x] > cpu->v[y]);
                    cpu->v[x] -= cpu->v[y];
                    break;

                case 6:
                    /*
                     * 8X06: SHR X
                     * Shifts right V[x]. Least significant bit from V[x]
                     * before shifting will be moved to V[15]. Thus, V[15]
                     * will be set to 1 if V[x] was odd before shifting.
                     */
                    cpu->v[0xF] = (cpu->v[x] & 1);
                    cpu->v[x] >>= 1;
                    break;

                case 7:
                    /*
                     * 8XY7: SUBN X, Y
                     * Substract V[x] from V[y] and store the result in V[x].
                     * V[15] is used as a borrow flag in the same sense than
                     * SUB X, Y did: V[15] is set to 0 if there is borrow,
                     * else to 1. Which is easier to check as if V[y] is
                     * greater than V[x].
                     */
                    cpu->v[0xF] = (cpu->v[y] > cpu->v[x]);
                    cpu->v[x] = cpu->v[y] - cpu->v[x];
                    break;

                case 0xE:
                    /*
                     * 8X0E: SHL X
                     * Shifts left V[x]. Most significant bit from V[x] before
                     * shifting will be moved to V[15].
                     */
                    cpu->v[0xF] = ((cpu->v[x] & 0x80) != 0);
                    cpu->v[x] <<= 1;
                    break;
            }
            break;

        case 9:
            /*
             * 9XY0: SNE X, Y
             * Skip next instruction if V[x] != V[y].
             */
            if (cpu->v[x] != cpu->v[y])
                cpu->pc = (cpu->pc + 2) & 0xFFF;
            break;
            
        case 0xA:
            /*
             * ANNN: LD I, NNN
             * Will set I register to NNN.
             */
            cpu->i = nnn;
            break;

        case 0xB:
            /*
             * BNNN: JP V0, NNN
             * Will jump to memory address (V[0] + NNN).
             */
            cpu->pc = (cpu->v[0] + nnn) & 0xFFF;
            break;

        case 0xC:
            /*
             * CXKK: RND X, KK
             * Will get a random value, then bitmasking it using KK as an
             * AND mask, and then save that random value to V[x].
             */
            cpu->v[x] = rand() & kk;
            break;

        case 0xD:
            /*
             * DXYN: DRW X, Y, N
             * Draw a sprite on screen. The sprite will be drawn on screen
             * position described by V[x],V[y]. Sprites are 8 pixels wide
             * and the number of rows to draw is indicated through N.
             * The sprite to draw is pointed using I register.
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
                /*
                 * EX9E: SKP X
                 * Skip next instruction if key indicated in V[x] is down.
                 */
                if (is_key_down(cpu->v[x]))
                    cpu->pc = (cpu->pc + 2) & 0xFFF;
            } else if (kk == 0xA1) {
                /*
                 * EXA1: SKNP X
                 * Skip next instruction if key indicated in V[x] is not down.
                 */
                if (!is_key_down(cpu->v[x]))
                    cpu->pc = (cpu->pc + 2) & 0xFFF;
            }
            break;

        case 0xF:
            switch (kk) {
                case 0x07:
                    /*
                     * FX07: LD X, DT
                     * Set V[x] to whatever is on DT register.
                     */
                    cpu->v[x] = cpu->dt;
                    break;

                case 0x0A:
                    /*
                     * FX0A: LD X, K
                     * Halt the machine until a key is pressed, then save
                     * the key number pressed in register V[x].
                     */
                    cpu->wait_key = x;
                    break;

                case 0x15:
                    /*
                     * FX15: LD DT, X
                     * Will set DT register to the value on V[x].
                     */
                    cpu->dt = cpu->v[x];
                    break;

                case 0x18:
                    /*
                     * FX18: LD ST, X
                     * Will set ST register to the value on V[x].
                     */
                    cpu->st = cpu->v[x];
                    break;

                case 0x1E:
                    /*
                     * FX1E: ADD I, X
                     * Add V[x] to whatever is on I register.
                     */
                    cpu->i += cpu->v[x];
                    break;
                    
                case 0x29:
                    /*
                     * FX29: LD F, X
                     * Will set I to the address location where the sprite
                     * for drawing the number in V[x] is.
                     */
                    cpu->i = 0x50 + (cpu->v[x] & 0xF) * 5;
                    break;

                case 0x33:
                    /*
                     * FX33: LD B, X
                     * Will set the value in memory address I, I+1 and I+2
                     * so that they represent the memory addresses for both
                     * hundreds, tens and ones for the number in V[x]
                     * register encoded in BCD.
                     */
                    cpu->mem[cpu->i + 2] = cpu->v[x] % 10;
                    cpu->mem[cpu->i + 1] = (cpu->v[x] / 10) % 10;
                    cpu->mem[cpu->i] = (cpu->v[x] / 100); 
                    break;

                case 0x55:
                    /*
                     * FX55: LD [I], X
                     * Will save in memory registers from V[0] to V[x] in
                     * memory addresses I to I+x. V[x] is included in what
                     * gets saved.
                     */
                    for (int reg = 0; reg <= x; reg++)
                        cpu->mem[cpu->i + reg] = cpu->v[reg];
                    break;

                case 0x65:
                    /*
                     * FX65: LD X, [I]
                     * Will read from memory addresses I to I+x and store
                     * each value in registers V[0] to V[x]. V[x] is included
                     * in what is read.
                     */
                    for (int reg = 0; reg <= x; reg++)
                        cpu->v[reg] = cpu->mem[cpu->i + reg];
                    break;
            }
            break;
    }
}

int
main(int argc, char** argv)
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    SDL_Surface* surface;
    SDL_Event event;
    struct machine_t mac;
    int mustQuit = 0;
    int last_ticks = 0;

    // Read ROM file to load.
    if (argc == 1) {
        fprintf(stderr, "Usage: %s ROMFILE\n", argv[0]);
        return 1;
    }

    // Init emulator
    init_machine(&mac);
    if (load_rom(argv[1], &mac)) {
        return 1;
    }
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
    expand_screen(mac.screen, (Uint32 *) surface->pixels);
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
            expand_screen(mac.screen, (Uint32 *) surface->pixels);
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
