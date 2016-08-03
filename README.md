# CHIP-8 [![Build Status](https://travis-ci.org/danirod/chip8.svg)](https://travis-ci.org/danirod/chip8) [![Coverage Status](https://coveralls.io/repos/github/danirod/chip8/badge.svg?branch=devel)](https://coveralls.io/github/danirod/chip8?branch=devel) [![GitHub tag](https://img.shields.io/github/tag/danirod/chip8.svg)](https://github.com/danirod/chip8/releases/latest) [![GitHub license](https://img.shields.io/badge/license-GPL3-blue.svg)](http://www.gnu.org/licenses/gpl-3.0.html)

chip8 is a CHIP-8 emulator developed in C using the SDL2 multimedia library. It emulates a standard CHIP-8 machine and implements all the opcodes that the specification provides.

## Usage

To emulate a binary ROM just provide the file as an argument:

```sh
$ chip8 ~/roms/TETRIS.bin
```

You can have more information by reading the [software manual](http://www.danirod.es/chip8/docs/current/manual/). [Download as PDF](http://www.danirod.es/chip8/docs/current/chip8.pdf).

## Building from sources

In order to compile this project you will need to have SDL 2.0 headers and
libraries in your machine. Head to www.libsdl.org to get those in case
you still haven't got them or get them using your package manager if your
operating system has any.

After installing SDL 2.0 you can download the software distribution and install it via the following commands:

```sh
autoreconf --install  # Only if you are using the Git repository
./configure
make
make install
make check  # Optional: to test the emulator -- libcheck is required
```

## Screenshots

GNU/Linux:

![CHIP-8 Emulator on GNU/Linux](http://www.danirod.es/chip8/screenshots/linux.png)

Apple® MacOS® X:

![CHIP-8 Emulator on MacOS X](http://www.danirod.es/chip8/screenshots/mac.png)


Microsoft® Windows®:

![CHIP-8 Emulator on Windows](http://www.danirod.es/chip8/screenshots/windows.png)

## License

    CHIP-8: A multiplatform CHIP-8 emulator done in SDL
    Copyright © 2015-2016 Dani Rodríguez <danirod@outlook.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

See COPYING for the entire contents of the license.
