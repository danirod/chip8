TESTS = chip8_test
check_PROGRAMS = chip8_test
chip8_test_SOURCES = test.c opchip.c opschip.c screen.c
chip8_test_CFLAGS = -std=c99 -Wall @CHECK_CFLAGS@ -I$(top_srcdir)/src
chip8_test_LDADD = @CHECK_LIBS@ $(top_srcdir)/src/lib8/lib8.a
