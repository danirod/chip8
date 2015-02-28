#include <stdio.h>
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

int main(int argc, const char * argv[])
{
    printf("Hello, World!\n");
    return 0;
}