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

#define MEMSIZ 4096 // How much memory can handle the CHIP-8

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
};

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
    machine->sp = machine->i = machine->dt = machine->st = 0;
    machine->pc = 0x200;

    memset(machine->mem, 0, MEMSIZ);
    memset(machine->stack, 0, 32);
    memset(machine->v, 0, 16);
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

int
main(int argc, const char * argv[])
{
    struct machine_t mac;
    
    init_machine(&mac);
    load_rom(&mac);

    int mustQuit = 0;
    while (!mustQuit) {
        // Read next opcode from memory.
        uint16_t opcode = (mac.mem[mac.pc] << 8) | mac.mem[mac.pc + 1];
        mac.pc = (mac.pc + 2) & 0xFFF;

        // Extract bit nibbles from the opcode
        uint16_t nnn = opcode & 0x0FFF;
        uint8_t kk = opcode & 0xFF;
        uint8_t n = opcode & 0xF;
        uint8_t x = (opcode >> 8) & 0xF;
        uint8_t y = (opcode >> 4) & 0xF;
        uint8_t p = (opcode >> 12);
        
        // INFERNAL SWITCH CASE! (Sorry for all the heart attacks here u_u)
        switch (p) {
            case 0:
                if (opcode == 0x00E0) {
                    printf("CLS\n");
                } else if (opcode == 0x00EE) {
                    printf("RET\n");
                }
                break;
            case 1:
                // JP nnn: set program counter to nnn
                mac.pc = nnn;
                break;
            case 2:
                printf("CALL %x\n", nnn);
                break;
            case 3:
                // SE x, kk: if V[x] == kk -> pc += 2
                if (mac.v[x] == kk)
                    mac.pc = (mac.pc + 2) & 0xFFF;
                break;
            case 4:
                // SNE x, kk: if V[x] != kk -> pc += 2
                if (mac.v[x] != kk)
                    mac.pc = (mac.pc + 2) & 0xFFF;
                break;
            case 5:
                // SE x, y: if V[x] == V[y] -> pc += 2
                if (mac.v[x] == mac.v[y])
                    mac.pc = (mac.pc + 2) & 0xFFF;
                break;
            case 6:
                // LD x, kk: V[x] = kk
                mac.v[x] = kk;
                break;
            case 7:
                // ADD x, kk: V[x] = (V[x] + kk) & 0xFF
                mac.v[x] = (mac.v[x] + kk) & 0xFF;
                break;
            case 8:
                switch (n) {
                    case 0:
                        // LD x, y: V[x] = V[y]
                        mac.v[x] = mac.v[y];
                        break;
                    case 1:
                        // OR x, y: V[x] = V[x] | V[y];
                        mac.v[x] |= mac.v[y];
                        break;
                    case 2:
                        // AND x, y: V[x] = V[x] & V[y]
                        mac.v[x] &= mac.v[y];
                        break;
                    case 3:
                        // XOR x, y: V[x] = V[x] ^ V[y]
                        mac.v[x] ^= mac.v[y];
                        break;
                    case 4:
                        // ADD x, y: V[x] += V[y]
                        mac.v[0xF] = (mac.v[x] > mac.v[x] + mac.v[y]);
                        mac.v[x] += mac.v[y];
                        break;
                    case 5:
                        // SUB x, y: V[x] -= V[y]
                        mac.v[0xF] = (mac.v[x] > mac.v[y]);
                        mac.v[x] -= mac.v[y];
                        break;
                    case 6:
                        // SHR x : V[x] = V[x] >> 1
                        mac.v[0xF] = (mac.v[x] & 1);
                        mac.v[x] >>= 1;
                        break;
                    case 7:
                        // SUBN x, y: V[x] = V[y] - V[x]
                        mac.v[0xF] = (mac.v[y] > mac.v[x]);
                        mac.v[x] = mac.v[y] - mac.v[x];
                        break;
                    case 0xE:
                        // SHL x : V[x] = V[x] << 1
                        mac.v[0xF] = ((mac.v[x] & 0x80) != 0);
                        mac.v[x] <<= 1;
                        break;
                }
                break;
            case 9:
                // SNE x, y: V[x] != V[y] -> pc += 2;
                if (mac.v[x] != mac.v[y])
                    mac.pc = (mac.pc + 2) & 0xFFF;
                break;
            case 0xA:
                // LD I, x : I = nnn
                mac.i = nnn;
                printf("LD I, %x\n", nnn);
                break;
            case 0xB:
                // JP V0, nnn: pc = V[0] + nnn
                mac.pc = mac.v[0] + nnn;
                break;
            case 0xC:
                printf("RND %x, %x\n", x, kk);
                break;
            case 0xD:
                printf("DRW %x, %x, %x\n", x, y, n);
                break;
            case 0xE:
                if (kk == 0x9E) {
                    printf("SKP %x\n", x);
                } else if (kk == 0xA1) {
                    printf("SKNP %x\n", x);
                }
                break;
            case 0xF:
                switch (kk) {
                    case 0x07:
                        // LD V[x], DT: V[x] = DT
                        mac.v[x] = mac.dt;
                        break;
                    case 0x0A:
                        printf("LD %x, K\n", x);
                        break;
                    case 0x15:
                        // LD DT, V[x] -> DT = V[x]
                        mac.dt = mac.v[x];
                        break;
                    case 0x18:
                        // LD ST, V[x] -> ST = V[x]
                        mac.st = mac.v[x];
                        break;
                    case 0x1E:
                        // ADD I, V[x] -> I += V[x]
                        mac.i += mac.v[x];
                        break;
                    case 0x29:
                        printf("LD F, %x\n", x);
                        break;
                    case 0x33:
                        printf("LD B, %x\n", x);
                        break;
                    case 0x55:
                        printf("LD [I], %x\n", x);
                        break;
                    case 0x65:
                        printf("LD %x, [I]\n", x);
                        break;
                }
                break;
        }

    }

    return 0;
}
