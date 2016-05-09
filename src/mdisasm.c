#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "../config.h"

#define APP_NAME "mdisasm"

#define OPCODE_P(opcode) (opcode >> 12)
#define OPCODE_X(opcode) ((opcode & 0xF00) >> 8)
#define OPCODE_Y(opcode) ((opcode & 0xF0) >> 4)
#define OPCODE_N(opcode) (opcode & 0xF)
#define OPCODE_NNN(opcode) (opcode & 0xFFF)
#define OPCODE_KK(opcode) (opcode & 0xFF)

static char* output_mode = "full";

static struct option long_options[] = {
    { "help", no_argument, 0, 'h' },
    { "version", no_argument, 0, 'v' },
    { "output", optional_argument, 0, 0 },
    { 0, 0, 0, 0 }
};

static void
to_instruction_0(unsigned short opcode, char* out)
{
    if (opcode == 0x00E0) {
        snprintf(out, 20, "EXIT");
    } else if (opcode == 0x00EE) {
        snprintf(out, 20, "RET");
    } else {
        snprintf(out, 20, "SYS %x", OPCODE_NNN(opcode));
    }
}

static void
to_instruction_1(unsigned short opcode, char* out)
{
    snprintf(out, 20, "JP %x", OPCODE_NNN(opcode));
}

static void
to_instruction_2(unsigned short opcode, char* out)
{
    snprintf(out, 20, "CALL %x", OPCODE_NNN(opcode));
}

static void
to_instruction_3(unsigned short opcode, char* out)
{
    snprintf(out, 20, "SE V[%x], %x", OPCODE_X(opcode), OPCODE_KK(opcode));
}

static void
to_instruction_4(unsigned short opcode, char* out)
{
    snprintf(out, 20, "SNE V[%x], %x", OPCODE_X(opcode), OPCODE_KK(opcode));
}

static void
to_instruction_5(unsigned short opcode, char* out)
{
    snprintf(out, 20, "SE V[%x], V[%x]", OPCODE_X(opcode), OPCODE_Y(opcode));
}

static void
to_instruction_6(unsigned short opcode, char* out)
{
    snprintf(out, 20, "LD V[%x], %x", OPCODE_X(opcode), OPCODE_KK(opcode));
}

static void
to_instruction_7(unsigned short opcode, char* out)
{
    snprintf(out, 20, "ADD V[%x], %x", OPCODE_X(opcode), OPCODE_KK(opcode));
}

static void
to_instruction_8(unsigned short opcode, char* out)
{
    int x = OPCODE_X(opcode), y = OPCODE_Y(opcode), n = OPCODE_N(opcode);
    switch (n) {
        case 0:
            snprintf(out, 20, "LD V[%x], V[%x]", x, y);
            break;
        case 1:
            snprintf(out, 20, "OR V[%x], V[%x]", x, y);
            break;
        case 2:
            snprintf(out, 20, "AND V[%x], V[%x]", x, y);
            break;
        case 3:
            snprintf(out, 20, "XOR V[%x], V[%x]", x, y);
            break;
        case 4:
            snprintf(out, 20, "ADD V[%x], V[%x]", x, y);
            break;
        case 5:
            snprintf(out, 20, "SUB V[%x], V[%x]", x, y);
            break;
        case 6:
            snprintf(out, 20, "SHR V[%x]", x);
            break;
        case 7:
            snprintf(out, 20, "SUBN V[%x], V[%x]", x, y);
            break;
        case 0xE:
            snprintf(out, 20, "SHL V[%x]", x);
            break;
    }
}

static void
to_instruction_9(unsigned short opcode, char* out)
{
    snprintf(out, 20, "SNE V[%x], V[%x]", OPCODE_X(opcode), OPCODE_Y(opcode));
}

static void
to_instruction_A(unsigned short opcode, char* out)
{
    snprintf(out, 20, "LD I, %x", OPCODE_NNN(opcode));
}

static void
to_instruction_B(unsigned short opcode, char* out)
{
    snprintf(out, 20, "JP V[0], %x", OPCODE_NNN(opcode));
}

static void
to_instruction_C(unsigned short opcode, char* out)
{
    snprintf(out, 20, "RND V[%x], %x", OPCODE_X(opcode), OPCODE_KK(opcode));
}

static void
to_instruction_D(unsigned short opcode, char* out)
{
    int x = OPCODE_X(opcode), y = OPCODE_Y(opcode), n = OPCODE_N(opcode);
    snprintf(out, 20, "DRW V[%x], V[%x], %x", x, y, n);
}

static void
to_instruction_E(unsigned short opcode, char* out)
{
    if (OPCODE_KK(opcode) == 0x9E) {
        snprintf(out, 20, "SKP V[%x]", OPCODE_X(opcode));
    } else if (OPCODE_KK(opcode) == 0xA1) {
        snprintf(out, 20, "SKNP V[%x]", OPCODE_X(opcode));
    }
}

static void
to_instruction_F(unsigned short opcode, char* out)
{
    switch (OPCODE_KK(opcode)) {
        case 0x07:
            snprintf(out, 20, "LD V[%x], DT", OPCODE_X(opcode));
            break;
        case 0x0A:
            snprintf(out, 20, "LD V[%x], K", OPCODE_X(opcode));
            break;
        case 0x15:
            snprintf(out, 20, "LD DT, V[%x]", OPCODE_X(opcode));
            break;
        case 0x18:
            snprintf(out, 20, "LD ST, V[%x]", OPCODE_X(opcode));
            break;
        case 0x1E:
            snprintf(out, 20, "ADD I, V[%x]", OPCODE_X(opcode));
            break;
        case 0x29:
            snprintf(out, 20, "LD F, V[%x]", OPCODE_X(opcode));
            break;
        case 0x33:
            snprintf(out, 20, "LD B, V[%x]", OPCODE_X(opcode));
            break;
        case 0x55:
            snprintf(out, 20, "LD [I], V[%x]", OPCODE_X(opcode));
            break;
        case 0x65:
            snprintf(out, 20, "LD V[%x], [I]", OPCODE_X(opcode));
            break;
    }
}

typedef void (*to_instr) (unsigned short opcode, char* out);

static to_instr instructions[] = {
    &to_instruction_0, &to_instruction_1, &to_instruction_2, &to_instruction_3,
    &to_instruction_4, &to_instruction_5, &to_instruction_6, &to_instruction_7,
    &to_instruction_8, &to_instruction_9, &to_instruction_A, &to_instruction_B,
    &to_instruction_C, &to_instruction_D, &to_instruction_E, &to_instruction_F
};

static void
to_instruction(unsigned short opcode, char* out)
{
    int p = OPCODE_P(opcode);
    // Fallback. In case instructions array fail to print for some reason.
    snprintf(out, 20, "%04x", opcode);
    instructions[p](opcode, out);
}

static int
load_rom_file(const char* filename, unsigned char** buf)
{
    // Try to open the file.
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) {
        fprintf(stderr, "File not found: %s\n", filename);
        return -1;
    }

    // How big is this file?
    fseek(fp, 0, SEEK_END);
    int length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    // Try to load the ROM.
    *buf = malloc(length);
    if (*buf == NULL) {
        return -1;
    }
    fread(*buf, length, 1, fp);
    fclose(fp);

    // Return how many bytes we read (is the size of the array).
    return length;
}

static void
usage(const char *app_name)
{
    printf("Usage: %s [-h] [-v] [--output=full|minimal] <file>\n", app_name);
}

int
main(int argc, char** argv)
{
    int indexpt, c;
    while ((c = getopt_long(argc, argv, "hv", long_options, &indexpt)) != -1) {
        switch (c) {
            case 'h':
                usage(argv[0]);
                exit(0);
            case 'v':
                printf("%s %s\n", APP_NAME, PACKAGE_VERSION);
                exit(0);
                break;
            case 0:
                if (!strncmp(long_options[indexpt].name, "output", 7)) {
                    output_mode = optarg;
                    if (output_mode == NULL) {
                        printf("--output: no option given.\n");
                        exit(1);
                    }
                    if (strncmp(output_mode, "full", 5) &&
                            strncmp(output_mode, "minimal", 8)) {
                        printf("Unknown value %s. Use %s -h\n",
                                output_mode, argv[0]);
                        exit(1);
                    }
                    break;
                }
                printf("Unknown option: %s. Use %s -h\n",
                        long_options[indexpt].name, argv[0]);
                break;
            default:
                exit(1);
        }
    }
    
    /* If this is true, no file has been given. */
    if (optind >= argc) {
        fprintf(stderr, "%1$s: no file given. '%1$s -h' for help.\n", argv[0]);
        exit(1);
    }

    unsigned char* buffer;
    int length = load_rom_file(argv[optind], &buffer);
    char output[20];

    for (int byte = 0; byte < length; byte += 2) {
        unsigned short opcode = buffer[byte] << 8 | buffer[byte + 1];
        to_instruction(opcode, output);
        if (!strncmp(output_mode, "full", 5)) {
            printf("%03x\t%04x\t%s\n", (0x200 + byte), opcode, output);
        } else if (!strncmp(output_mode, "minimal", 8)) {
            printf("%s\n", output);
        }
    }

    free(buffer);
    return 0;
}
