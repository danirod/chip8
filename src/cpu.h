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

#ifndef CPU_H_
#define CPU_H_

#include <stdint.h>

#define MEMSIZ 4096 // How much memory can handle the CHIP-8

/**
 * Main data structure for holding information and state about processor.
 * Memory, stack, and register set is all defined here.
 */
struct machine_t
{
    uint8_t  mem[MEMSIZ];       // Memory is allocated as a buffer
    uint16_t pc;                // Program Counter

    uint16_t stack[16];         // Stack can hold 16 16-bit values
    uint16_t sp;                // Stack pointer

    uint8_t  v[16];             // 16 general purpose registers
    uint16_t i;                 // Special I register
    uint8_t  dt, st;            // Timers

    char screen[2048];          // Screen bitmap
    char wait_key;              // Key the CHIP-8 is idle waiting for.
};

typedef void (*procesarInstruccionesOpcodes)(struct machine_t*, uint16_t, uint16_t,
                                             uint8_t, uint8_t, uint8_t, uint8_t,
                                             uint8_t);
procesarInstruccionesOpcodes instruccion_p;
procesarInstruccionesOpcodes instruccion_n;
procesarInstruccionesOpcodes instruccion_kk;
 
void init_machine(struct machine_t*);

void step_machine(struct machine_t*);

/*===================================================================================================                       
 *=============    Nomenclatura para comprender el nombre de las fuciones añadidas:    ==============                       
 *===================================================================================================                       
 *===    void spC1_1NNN (...);                                                                    ===                       
 *===                                                                                             ===                       
 *===    sp = "s" se refiere a switch, "p" a la expresion que se usaba           ej: switch (p) { ===                       
 *===    C1 = "C" se refiere a case,   "1" a la expresion constante que se usaba ej: case 1 :     ===                       
 *===                                                                                             ===                       
 *===    _1NNN = "_" se usa para que sea mas legible,                                             ===                       
 *===            "1NNN" se refiere a las instrucciones que maneja dentro,                         ===                       
 *===            si ahy varias, se reflejaran las que consten separandolas,                       ===                       
 *===            por "_", a no ser que anteriormente se manejaran mediante,                       ===                       
 *===            un switch, en ese caso se pondria la nomenclatura                                ===                       
 *===            para el switch cita anteriormente.                                               ===                       
 *===                                                                                             ===                       
 *===    ej: para dos instrucciones:                                                              ===                       
 *===        spC0_00E0_00EE                                                                       ===                       
 *===                                                                                             ===                       
 *===    ej: para una situacion en la que anteriormente se usaba in switch anidado:               ===                       
 *===        spC8_sn.                                                                             ===                       
 *===                                                                                             ===                       
 *===                                                                                             ===                       
 *===        Nota: Ahora se manejara dentro otro array de punteros a funciones.                   ===                       
 *===================================================================================================*/


//instrucciones que usaba p en el antiguo switch, y la entrada al antiguo switch n.                                          
void spC0_00E0_00EE   (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t); //00E0: CLS "or" 00EE: RET "in if"                               
void spC1_1NNN        (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t); //1NNN: JMP  NNN  

void spC2_2NNN        (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t); //2NNN: CALL NNN                                                 
void spC3_3XKK        (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t); //3XKK: SE   X, KK                                               
void spC4_4XKK        (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t); //4XKK: SNE  X, KK                                               
void spC5_5XY0        (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t); //5XY0: SE   X, Y                                                
void spC6_6XKK        (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t); //6XKK: LD   X, KK                                               
void spC7_7XKK        (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t); //7XKK: ADD  X, KK                                               
void spC8_sn          (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t);

void spC9_9XY0        (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t); //9XY0: SNE X, Y  

void spC0xA_ANNN      (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t);  //ANNN: LD I, NNN                                               
void spC0xB_BNNN      (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t);  //BNNN: JP V0, NNN                                              
void spC0xC_CXKK      (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t);  //CXKK: RND X, KK                                               
void spC0xD_DXYN      (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t);  //DXYN: X, Y, N                                                 
void spC0xE_EX9E_EXA1 (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t);  //EX9E: SKP X "or" EXA1: SKNP X "in if"                         
void spC0xF_skk       (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t);

//instrucciones que usaba n en el antiguo swich                                                                              
void snC0_8XY0        (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t); //8XY0: LD   X, Y 

void snC1_8XY1        (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t); //8XY1: OR   X, Y                                                
void snC2_8XY2        (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t); //8XY2: AND  X, Y                                                
void snC3_8XY3        (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t); //8XY3: XOR  X, Y                                                
void snC4_8XY4        (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t); //8XY4: ADD  X, Y                                                
void snC5_8XY5        (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t); //8XY5: SUB  X, Y                                                
void snC6_8X06        (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t); //8X06: SHR  X                                                   
void snC7_8XY7        (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t); //8XY7: SUBN X, Y                                                
void snC0xE_8X0E      (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t); //8X0E: SHL  X 

//instrucciones que usaba kk en el antiguo swich                                                                             
void skkC0x07_FX07    (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t); //FX07: LD X,   DT                                               
void skkC0x0A_FX0A    (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t); //FX0A: LD X,   K                                                
void skkC0x15_FX15    (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t); //FX15: LD DT,  X                                                
void skkC0x18_FX18    (struct machine_t*, uint16_t, uint16_t, uint8_t,
		       uint8_t, uint8_t, uint8_t, uint8_t); //FX18: LD ST,  X                                                
void skkC0x1E_FX1E    (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t); //FX1E: ADD I,  X                                                

void skkC0x29_FX29    (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t); //FX29: LD F,   X                                                
void skkC0x33_FX33    (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t); //FX33: LD B,   X                                                
void skkC0x55_FX55    (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t); //FX55: LD [I], X                                                
void skkC0x65_FX65    (struct machine_t*, uint16_t, uint16_t, uint8_t,
                       uint8_t, uint8_t, uint8_t, uint8_t); //FX65: LD X,   [I]  

#endif // CPU_H_
