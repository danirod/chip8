noinst_LIBRARIES = lib8.a
lib8_a_SOURCES = cpu.c cpu.h
lib8_a_CFLAGS = -std=c99 -Wall

bin_PROGRAMS = chip8
chip8_SOURCES = chip8.c sdl.c sdl.h
chip8_CFLAGS = @SDL_CFLAGS@ -std=c99 -Wall
chip8_LDADD = lib8.a @SDL_LIBS@
