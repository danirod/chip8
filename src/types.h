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

#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>

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

#define ADDRESS_MASK 0xFFF

#endif // TYPES_H_
