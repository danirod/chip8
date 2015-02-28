#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MEMSIZ 4096

/**
 * Estructura de datos para representar el estado del procesador.
 * La memoria, la pila y todos los registros usados por la máquina están
 * representados aquí.
 */
struct machine_t
{
    uint8_t mem[MEMSIZ];        // Banco de memoria disponible para la CPU
    uint16_t pc;                // Contador de programa
    
    uint16_t stack[16];         // Pila. 16 registros de 16 bits
    uint16_t sp;                // Puntero de pila
    
    uint8_t v[16];              // 16 registros de propósito general
    uint16_t i;                 // Registro especial de dirección I
    uint8_t dt, st;             // Temporizadores
};

void
init_machine(struct machine_t* machine)
{
    machine->sp = machine->i = machine->dt = machine->st = 0x00;
    machine->pc = 0x200;
    for (int i = 0; i < MEMSIZ; i++)
        machine->mem[i] = 0x00;
    for (int i = 0; i < 16; i++) {
        machine->stack[i] = 0;
        machine->v[i] = 0;
    }
}

void
load_rom(struct machine_t* machine)
{
    FILE* fp = fopen("PONG", "r");
    if (fp == NULL) {
        fprintf(stderr, "Cannot open ROM file.\n");
        exit(1);
    }
    
    // Obtengo el tamaño del archivo.
    fseek(fp, 0, SEEK_END);
    int length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    fread(machine->mem + 0x200, length, 1, fp);

    fclose(fp);
}

int main(int argc, const char * argv[])
{
    struct machine_t mac;
    init_machine(&mac);
    load_rom(&mac);
    int mustQuit = 0;
    while (!mustQuit) {
        // Leer el opcode
        uint16_t opcode = (mac.mem[mac.pc] << 8) | mac.mem[mac.pc + 1];
        mac.pc += 2;
        if (mac.pc == MEMSIZ)
            mac.pc = 0;

        uint16_t nnn = opcode & 0x0FFF;
        uint8_t kk = opcode & 0xFF;
        uint8_t n = opcode & 0xF;
        uint8_t x = (opcode >> 8) & 0xF;
        uint8_t y = (opcode >> 4) & 0xF;
        uint8_t p = (opcode >> 12);

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
