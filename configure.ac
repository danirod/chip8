# configure.ac

AC_INIT([chip8], [0.1.3], [danirod@outlook.com])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

# Check programs
AM_PROG_AR
AC_PROG_CC
AC_PROG_RANLIB

# Check libraries
AC_CHECK_LIB([m], [sinf], [], [AC_MSG_ERROR(["** ERROR: Math library not found **"])])
# Check header files

# Check typedefs, structures and so

# Check library functions
AM_PATH_SDL2([2.0.0], :, AC_MSG_ERROR(["** ERROR: SDL 2.0 is required **"]))
PKG_CHECK_MODULES([CHECK], [check >= 0.9.6])

# Output
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile])
AC_OUTPUT
