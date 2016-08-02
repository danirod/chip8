SUBDIRS = lib8 .

bin_PROGRAMS = chip8
chip8_SOURCES = chip8.c libsdl.c libsdl.h
chip8_CFLAGS = @SDL_CFLAGS@ -std=c99 -Wall
chip8_LDADD = lib8.a @SDL_LIBS@
