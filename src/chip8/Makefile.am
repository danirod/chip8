# This Makefile builds the CHIP-8 emulator.

bin_PROGRAMS = chip8
chip8_SOURCES = chip8.c libsdl.c libsdl.h
chip8_CFLAGS = -I$(top_srcdir)/src @SDL_CFLAGS@ -std=c99 -Wall
chip8_LDADD = $(top_srcdir)/src/lib8/lib8.a @SDL_LIBS@
dist_man_MANS = chip8.1
