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
#include <stdlib.h>
#include <string.h>

/**
 * These are the bitmaps for the sprites that represent numbers.
 * This array should be memcopied to memory address 0x050. LD F, Vx
 * instruction sets I register to the memory address of a provided
 * number.
 */
static char hexcodes[] = {
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

/* Fxxx Opcodes. */

static void
skkC0x07_FX07 (struct machine_t* cpu, word opcode, word nnn,
	       byte kk, byte n, byte x, byte y, byte p){
      /*
       * FX07: LD X, DT
       * Set V[x] to whatever is on DT register.
       */
      cpu->v[x] = cpu->dt;
}

static void
skkC0x0A_FX0A (struct machine_t* cpu, word opcode, word nnn,
	       byte kk, byte n, byte x, byte y, byte p){
      /*
       * FX0A: LD X, K
       * Halt the machine until a key is pressed, then save
       * the key number pressed in register V[x].
       */
      cpu->wait_key = x;
}

static void
skkC0x15_FX15 (struct machine_t* cpu, word opcode, word nnn,
	       byte kk, byte n, byte x, byte y, byte p){
      /*
       * FX15: LD DT, X
       * Will set DT register to the value on V[x].
       */
      cpu->dt = cpu->v[x];
}

static void
skkC0x18_FX18 (struct machine_t* cpu, word opcode, word nnn,
	       byte kk, byte n, byte x, byte y, byte p){
   /*
    * FX18: LD ST, X
    * Will set ST register to the value on V[x].
    */
  cpu->st = cpu->v[x];
}

static void
skkC0x1E_FX1E (struct machine_t* cpu, word opcode, word nnn,
	       byte kk, byte n, byte x, byte y, byte p){
      /*
       * FX1E: ADD I, X
       * Add V[x] to whatever is on I register.
       */
      cpu->i += cpu->v[x];
}
 
static void
skkC0x29_FX29 (struct machine_t* cpu, word opcode, word nnn,
	       byte kk, byte n, byte x, byte y, byte p){
      /*
       * FX29: LD F, X
       * Will set I to the address location where the sprite
       * for drawing the number in V[x] is.
       */
      cpu->i = 0x50 + (cpu->v[x] & 0xF) * 5;
}

static void
skkC0x33_FX33 (struct machine_t* cpu, word opcode, word nnn,
	       byte kk, byte n, byte x, byte y, byte p){
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
}

static void
skkC0x55_FX55 (struct machine_t* cpu, word opcode, word nnn,
	       byte kk, byte n, byte x, byte y, byte p){
      /*
       * FX55: LD [I], X
       * Will save in memory registers from V[0] to V[x] in
       * memory addresses I to I+x. V[x] is included in what
       * gets saved.
       */
      for (int reg = 0; reg <= x; reg++)
	cpu->mem[cpu->i + reg] = cpu->v[reg];
}

static void
skkC0x65_FX65 (struct machine_t* cpu, word opcode, word nnn,
	       byte kk, byte n, byte x, byte y, byte p){
      /*
       * FX65: LD X, [I]
       * Will read from memory addresses I to I+x and store
       * each value in registers V[0] to V[x]. V[x] is included
       * in what is read.
       */
      for (int reg = 0; reg <= x; reg++)
	cpu->v[reg] = cpu->mem[cpu->i + reg];
}

procesarInstruccionesOpcodes ptrFunInstrucciones_kk[9] = {&skkC0x07_FX07,
                                            &skkC0x0A_FX0A,
                                            &skkC0x15_FX15,
                                            &skkC0x18_FX18,
                                            &skkC0x1E_FX1E,
                                            &skkC0x29_FX29,
                                            &skkC0x33_FX33,
                                            &skkC0x55_FX55,
					    &skkC0x65_FX65};

/* 8xyp opcodes. */

static void
snC0_8XY0 (struct machine_t* cpu, word opcode, word nnn,
	   byte kk, byte n, byte x, byte y, byte p){
  /*
   * 8XY0: LD X, Y
   * Set V[x] = V[y]
   */
  cpu->v[x] = cpu->v[y];
}

static void
snC1_8XY1 (struct machine_t* cpu, word opcode, word nnn,
	   byte kk, byte n, byte x, byte y, byte p){
  /*
   * 8XY1: OR X, Y
   * Set V[x] to V[x] OR V[y].
   */
  cpu->v[x] |= cpu->v[y];
}

static void
snC2_8XY2 (struct machine_t* cpu, word opcode, word nnn,
	   byte kk, byte n, byte x, byte y, byte p){
      /*
       * 8XY2: AND X, Y
       * Set V[x] to V[x] AND V[y].
       */
      cpu->v[x] &= cpu->v[y];
} 

static void
snC3_8XY3 (struct machine_t* cpu, word opcode, word nnn,
	   byte kk, byte n, byte x, byte y, byte p){
      /*
       * 8XY3: XOR X, Y
       * Set V[x] to V[x] XOR V[y]
       */
      cpu->v[x] ^= cpu->v[y];
}

static void
snC4_8XY4 (struct machine_t* cpu, word opcode, word nnn,
	   byte kk, byte n, byte x, byte y, byte p){
      /*
       * 8XY4: ADD X, Y
       * Add V[y] to V[x]. V[15] is used as carry flag: if
       * there is a carry, V[15] must be set to 1, else to 0.
       */
      cpu->v[0xf] = (cpu->v[x] > cpu->v[x] + cpu->v[y]);
      cpu->v[x] += cpu->v[y];
} 

static void
snC5_8XY5 (struct machine_t* cpu, word opcode, word nnn,
	   byte kk, byte n, byte x, byte y, byte p){
      /*
       * 8XY5: SUB X, Y
       * Substract V[y] from V[x]. V[15] is used as borrow flag:
       * if there is a borrow, V[15] must be set to 0, else
       * to 1. Which in practice is easier to check as if
       * V[x] is greater than V[y].
       */
      cpu->v[0xF] = (cpu->v[x] > cpu->v[y]);
      cpu->v[x] -= cpu->v[y];
}

static void
snC6_8X06 (struct machine_t* cpu, word opcode, word nnn,
	   byte kk, byte n, byte x, byte y, byte p){
      /*
//---> esta mal escrito en vez de 0 va Y o es asi?       * 8X06: SHR X
       * Shifts right V[x]. Least significant bit from V[x]
       * before shifting will be moved to V[15]. Thus, V[15]
       * will be set to 1 if V[x] was odd before shifting.
       */
      cpu->v[0xF] = (cpu->v[x] & 1);
      cpu->v[x] >>= 1;
}

static void
snC7_8XY7 (struct machine_t* cpu, word opcode, word nnn,
	   byte kk, byte n, byte x, byte y, byte p){
      /*
       * 8XY7: SUBN X, Y
       * Substract V[x] from V[y] and store the result in V[x].
       * V[15] is used as a borrow flag in the same sense than
       * SUB X, Y did: V[15] is set to 0 if there is borrow,
       * else to 1. Which is easier to check as if V[y] is
       * greater than V[x].
       */
      cpu->v[0xF] = (cpu->v[y] > cpu->v[x]);
      cpu->v[x] = cpu->v[y] - cpu->v[x];
}

static void
snC0xE_8X0E (struct machine_t* cpu, word opcode, word nnn,
	     byte kk, byte n, byte x, byte y, byte p){
      /*
       * 8X0E: SHL X
       * Shifts left V[x]. Most significant bit from V[x] before
       * shifting will be moved to V[15].
       */
      cpu->v[0xF] = ((cpu->v[x] & 0x80) != 0);
      cpu->v[x] <<= 1;
}

procesarInstruccionesOpcodes ptrFunInstrucciones_n[9] = {&snC0_8XY0,
					   &snC1_8XY1,
					   &snC2_8XY2,
					   &snC3_8XY3,
					   &snC4_8XY4,
					   &snC5_8XY5,
					   &snC6_8X06,
					   &snC7_8XY7,
					   &snC0xE_8X0E};

static void
nibble_0(struct machine_t* cpu, word opcode, word nnn,
        byte kk, byte n, byte x, byte y, byte p)
{
    if (opcode == 0x00e0) {
        /* 00E0: CLS - Clear the screen. */
        memset(cpu->screen, 0, 2048);
    } else if (opcode == 0x00ee) {
        /* 00EE: RET - Return from subroutine. */
        if (cpu->sp > 0)
        cpu->pc = cpu->stack[--cpu->sp];
        /* TODO: Should throw an error on stack underflow. */
    }
}

static void
nibble_1(struct machine_t* cpu, word opcode, word nnn,
        byte kk, byte n, byte x, byte y, byte p)
{
    /* 1NNN: JMP - Jump to address location NNN. */
    cpu->pc = nnn;
}

static void
nibble_2(struct machine_t* cpu, word opcode, word nnn,
        byte kk, byte n, byte x, byte y, byte p)
{
    /* 2NNN: CALL - Call subroutine starting at address NNN. */
    if (cpu->sp < 16) {
        cpu->stack[cpu->sp++] = cpu->pc;
        cpu->pc = nnn;
    }
    /* TODO: Should throw an error on stack overflow. */
}

static void
nibble_3(struct machine_t* cpu, word opcode, word nnn,
        byte kk, byte n, byte x, byte y, byte p)
{
    /* 3XKK: SE: Skip next instruction if V[X] = KK. */
    if (cpu->v[x] == kk)
        cpu->pc = (cpu->pc + 2) & 0xfff;
}

static void
nibble_4(struct machine_t* cpu, word opcode, word nnn,
        byte kk, byte n, byte x, byte y, byte p)
{
    /* 4XKK: SNE - Skip next instruction if V[X] != KK. */
    if (cpu->v[x] != kk)
        cpu->pc = (cpu->pc + 2) & 0xfff;
}

static void
nibble_5(struct machine_t* cpu, word opcode, word nnn,
        byte kk, byte n, byte x, byte y, byte p)
{
    /* 5XY0: SE - Skip next instruction if V[X] == V[Y]. */
    if (cpu->v[x] == cpu->v[y])
        cpu->pc = (cpu->pc + 2) & 0xfff;
}

static void
nibble_6(struct machine_t* cpu, word opcode, word nnn,
        byte kk, byte n, byte x, byte y, byte p)
{
    /* 6XKK: LD - Set V[X] = KK. */
    cpu->v[x] = kk;
}

static void
nibble_7(struct machine_t* cpu, word opcode, word nnn,
        byte kk, byte n, byte x, byte y, byte p)
{
    /* 7XKK: ADD - Add KK to V[X]. */
    cpu->v[x] = (cpu->v[x] + kk) & 0xff;
}

static void
nibble_8(struct machine_t* cpu, word opcode, word nnn,
        byte kk, byte n, byte x, byte y, byte p)
{
    if (!(n < 0 || n > 7)) {
        instruccion_n = ptrFunInstrucciones_n[n];
        instruccion_n(cpu, opcode, nnn, kk, n, x, y, p);
    } else if (n == 0xE) {
        instruccion_n = ptrFunInstrucciones_n[n];
        instruccion_n(cpu, opcode, nnn, kk, n, x, y, p);
    }
}

static void
nibble_9(struct machine_t* cpu, word opcode, word nnn,
        byte kk, byte n, byte x, byte y, byte p)
{
    /* 9XY0: SNE - Skip next instruction if V[X] != V[Y]. */
    if (cpu->v[x] != cpu->v[y])
        cpu->pc = (cpu->pc + 2) & 0xFFF;
}

static void
nibble_A(struct machine_t* cpu, word opcode, word nnn,
        byte kk, byte n, byte x, byte y, byte p)
{
    /* ANNN: LD - Set I to NNN. */
    cpu->i = nnn;
}

static void
nibble_B(struct machine_t* cpu, word opcode, word nnn,
        byte kk, byte n, byte x, byte y, byte p)
{
    /* BNNN: JP - Jump to memory address (V[0] + NNN). */
    cpu->pc = (cpu->v[0] + nnn) & 0xFFF;
}

static void
nibble_C(struct machine_t* cpu, word opcode, word nnn,
        byte kk, byte n, byte x, byte y, byte p)
{
    /* CXKK: RND - Put a random value, bitmasked against KK in V[X]. */
    cpu->v[x] = rand() & kk;
}

static void
nibble_D(struct machine_t* cpu, word opcode, word nnn,
        byte kk, byte n, byte x, byte y, byte p)
{
    /* DXYN: DRW - Draw a sprite on the screen at location V[X], V[Y]. */
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
}

static void
nibble_E(struct machine_t* cpu, word opcode, word nnn,
        byte kk, byte n, byte x, byte y, byte p)
{
    if (kk == 0x9E) {
        /* EX9E: SKP - Skip next instruction if key V[X] is down. */
        if (cpu->poller(cpu->v[x]))
            cpu->pc = (cpu->pc + 2) & 0xFFF;
    } else if (kk == 0xA1) {
        /* EXA1: SKNP - Skip next instruction if key V[X] is not down. */
        if (!cpu->poller(cpu->v[x]))
            cpu->pc = (cpu->pc + 2) & 0xFFF;
    }
}

static void
nibble_F(struct machine_t* cpu, word opcode, word nnn,
        byte kk, byte n, byte x, byte y, byte p)
{
    switch (kk) {
    case 0x07:
        instruccion_kk = ptrFunInstrucciones_kk[0];
        instruccion_kk(cpu, opcode, nnn, kk, n, x, y, p);
        break;
    case 0x0A:
        instruccion_kk = ptrFunInstrucciones_kk[1];
        instruccion_kk(cpu, opcode, nnn, kk, n, x, y, p);
        break;
    case 0x15:
        instruccion_kk = ptrFunInstrucciones_kk[2];
        instruccion_kk(cpu, opcode, nnn, kk, n, x, y, p);
        break;
    case 0x18:
        instruccion_kk = ptrFunInstrucciones_kk[3];
        instruccion_kk(cpu, opcode, nnn, kk, n, x, y, p);
        break;
    case 0x1E:
        instruccion_kk = ptrFunInstrucciones_kk[4];
        instruccion_kk(cpu, opcode, nnn, kk, n, x, y, p);
        break;
    case 0x29:
        instruccion_kk = ptrFunInstrucciones_kk[5];
        instruccion_kk(cpu, opcode, nnn, kk, n, x, y, p);
        break;
    case 0x33:
        instruccion_kk = ptrFunInstrucciones_kk[6];
        instruccion_kk(cpu, opcode, nnn, kk, n, x, y, p);
        break;
    case 0x55:
        instruccion_kk = ptrFunInstrucciones_kk[7];
        instruccion_kk(cpu, opcode, nnn, kk, n, x, y, p);
        break;
    case 0x65:
        instruccion_kk = ptrFunInstrucciones_kk[8];
        instruccion_kk(cpu, opcode, nnn, kk, n, x, y, p);
        break;
    }
}

procesarInstruccionesOpcodes ptrFunInstrucciones_p[16] = {
    &nibble_0, &nibble_1, &nibble_2, &nibble_3,
    &nibble_4, &nibble_5, &nibble_6, &nibble_7,
    &nibble_8, &nibble_9, &nibble_A, &nibble_B,
    &nibble_C, &nibble_D, &nibble_E, &nibble_F
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

    instruccion_p = ptrFunInstrucciones_p[p];
    instruccion_p(cpu, opcode, nnn, kk, n, x, y, p);
}
