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

#ifndef ROM_H_
#define ROM_H_

#include "cpu.h"

/**
 * Load a hex file.
 *
 * @param file file path.
 * @param machine data structure to load the HEX into.
 */
int load_hex(const char* file, struct machine_t* machine);

/**
 * Load a ROM into a machine. This function will open a file and load its
 * contents into the memory from the provided machine data structure.
 * In compliance with the specification, ROM data will start at 0x200.
 *
 * @param file file path.
 * @param machine machine data structure to load the ROM into.
 */
int load_rom(const char* file, struct machine_t* machine);

#endif // ROM_H_
