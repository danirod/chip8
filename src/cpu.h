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

#ifndef CPU_H_
#define CPU_H_

#include <stdint.h>

#define MEMSIZ 4096 // How much memory can handle the CHIP-8

/**
 * Type definition for a byte value. Bytes are unsigned 8-bit variables.
 * They are widely used on the CHIP-8 since the memory and registers are
 * byte-sized.
 */
typedef uint8_t byte;

/**
 * Type definition for a word value. Words are unsigned 16-bit variables.
 * Words are used for decoding opcodes, since a opcode is 16-bit long.
 */
typedef uint16_t word;

/**
 * Type definition for refering to addresses. Addresses are 12-bit value.
 * However there is no such type on C, thus a 16-bit value is used and only
 * the 12 least significant bits are used.
 *
 * Address should be used whenever a memory address is being manipulated.
 * They shouldn't be used on any other situation.
 */
typedef uint16_t address;

/**
 * This is the maximum amount of memory addressable by the machine.
 * It should be used as a bitmask when overflows could possibly happen
 * due to memory address management.
 */
#define ADDRESS_MASK 0xFFF

typedef int (*keyboard_poller_t)(char);

/**
 * Main data structure for holding information and state about processor.
 * Memory, stack, and register set is all defined here.
 */
struct machine_t
{
    byte mem[MEMSIZ];         // Memory is allocated as a buffer
    address pc;                // Program Counter

    address stack[16];          // Stack can hold 16 16-bit values
    char sp;                    // Stack Pointer: points to next free cell

    byte v[16];              // 16 general purpose registers
    address i;                 // Special I register
    byte dt, st;             // Timers

    char screen[2048];          // Screen bitmap
    char wait_key;              // Key the CHIP-8 is idle waiting for.
    keyboard_poller_t poller; // Keyboard poller
};

void init_machine(struct machine_t*);

void step_machine(struct machine_t*);

typedef void (*procesarInstruccionesOpcodes)(struct machine_t*, word, word,
                                             byte, byte, byte, byte,
                                             byte);
procesarInstruccionesOpcodes instruccion_p;
procesarInstruccionesOpcodes instruccion_n;
procesarInstruccionesOpcodes instruccion_kk;
 
#endif // CPU_H_
