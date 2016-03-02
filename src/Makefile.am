noinst_LIBRARIES = lib8.a
lib8_a_SOURCES = cpu.c cpu.h

AM_CFLAGS = --std=c99 -Wall
bin_PROGRAMS = chip8
chip8_SOURCES = chip8.c sdl.c sdl.h
chip8_LDADD = lib8.a
