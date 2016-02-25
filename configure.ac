# configure.ac

AC_INIT([chip8], [0.1.1], [danirod@outlook.com])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

# Check programs
AC_PROG_CC

# Check libraries
AC_CHECK_LIB([SDL2], [SDL_Init], [], 
             [AC_MSG_ERROR(["** ERROR: SDL2 is required to build this. ** "])])
AC_CHECK_LIB([m], [sinf], [], [AC_MSG_ERROR(["** ERROR: Math library not found **"])])
# Check header files

# Check typedefs, structures and so

# Check library functions
PKG_CHECK_MODULES([CHECK], [check >= 0.9.6])

# Output
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile])
AC_OUTPUT

