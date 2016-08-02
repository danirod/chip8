#include <stdio.h>
#include <stdlib.h>

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

int
main(int argc, char** argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [file name]\n", argv[0]);
        return 1;
    }

    unsigned char* buffer;
    int length = load_rom_file(argv[1], &buffer);

    for (int byte = 0; byte < length; byte += 2) {
        printf("%02x%02x\n", buffer[byte], buffer[byte + 1]);
    }

    free(buffer);
    return 0;
}
