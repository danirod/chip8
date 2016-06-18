TESTS = chip8_test
check_PROGRAMS = chip8_test
chip8_test_SOURCES = test.c opchip.c opschip.c screen.c
chip8_test_CFLAGS = -std=c99 -Wall @CHECK_CFLAGS@
chip8_test_LDADD = @CHECK_LIBS@ ../src/lib8.a
