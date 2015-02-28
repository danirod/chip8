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
    machine->sp = machine->i = machine->dt = machine->st = 0x00;
    machine->pc = 0x200;

    // FIXME: Replace these for-loops with calls to memset(3).
    // Would do this right now but I don't want to do this off-camera!
    for (int i = 0; i < MEMSIZ; i++)
        machine->mem[i] = 0x00;
    for (int i = 0; i < 16; i++) {
        machine->stack[i] = 0;
        machine->v[i] = 0;
    }
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
        // FIXME: Change to mac.pc = (mac.pc + 2) & 0xFFF
        //        (Did I already said that I don't want to code off-camera?)
        mac.pc += 2;
        if (mac.pc == MEMSIZ)
            mac.pc = 0;

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
                printf("JP %x\n", nnn);
                break;
            case 2:
                printf("CALL %x\n", nnn);
                break;
            case 3:
                printf("SE %x, %x\n", x, kk);
                break;
            case 4:
                printf("SNE %x, %x\n", x, kk);
                break;
            case 5:
                printf("SE %x, %x\n", x, y);
                break;
            case 6:
                printf("LD %x, %x\n", x, kk);
                break;
            case 7:
                printf("ADD %x, %x\n", x, kk);
                break;
            case 8:
                switch (n) {
                    case 0:
                        printf("LD %x, %x\n", x, y);
                        break;
                    case 1:
                        printf("OR %x, %x\n", x, y);
                        break;
                    case 2:
                        printf("AND %x, %x\n", x, y);
                        break;
                    case 3:
                        printf("XOR %x, %x\n", x, y);
                        break;
                    case 4:
                        printf("ADD %x, %x\n", x, y);
                        break;
                    case 5:
                        printf("SUB %x, %x\n", x, y);
                        break;
                    case 6:
                        printf("SHR %x\n", x);
                        break;
                    case 7:
                        printf("SUBN %x, %x\n", x, y);
                        break;
                    case 0xE:
                        printf("SHL %x\n", x);
                        break;
                }
                break;
            case 9:
                printf("SNE %x, %x\n", x, y);
                break;
            case 0xA:
                printf("LD I, %x\n", nnn);
                break;
            case 0xB:
                printf("JP V0, %x\n", nnn);
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
                        printf("LD %x, DT\n", x);
                        break;
                    case 0x0A:
                        printf("LD %x, K\n", x);
                        break;
                    case 0x15:
                        printf("LD DT, %x\n", x);
                        break;
                    case 0x18:
                        printf("LD ST, %x\n", x);
                        break;
                    case 0x1E:
                        printf("ADD I, %x\n", x);
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
