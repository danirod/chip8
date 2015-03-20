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
#include "display.h"
#include "keyboard.h"
#include <stdlib.h>
#include <string.h>

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
    word opcode = (cpu->mem[cpu->pc] << 8) | cpu->mem[cpu->pc + 1];
    cpu->pc = (cpu->pc + 2) & 0xFFF;

    // Extract bit nibbles from the opcode
    word nnn = opcode & 0x0FFF;
    byte kk = opcode & 0xFF;
    byte n = opcode & 0xF;
    byte x = (opcode >> 8) & 0xF;
    byte y = (opcode >> 4) & 0xF;
    byte p = (opcode >> 12);

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
                byte sprite = cpu->mem[cpu->i + j];
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
