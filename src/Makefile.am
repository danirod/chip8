AM_CFLAGS = --std=c99 -I$(top_srcdir)/lib8

bin_PROGRAMS = chip8
chip8_SOURCES = chip8.c \
				display.c \
				display.h \
				keyboard.c \
				keyboard.h \
				sdl.c \
				sdl.h \
				sound.c \
				sound.h \
				rom.c \
				rom.h
chip8_LDADD = $(top_srcdir)/lib8/lib8.a
