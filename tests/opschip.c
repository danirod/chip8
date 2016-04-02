/*
 * chip8 is a CHIP-8 emulator done in C
 * Copyright (C) 2015-2016 Dani Rodríguez <danirod@outlook.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * File: tests/opschip.c
 * Description: Unit test related to SUPER-CHIP Opcodes.
 */

#include <check.h>
#include <stdint.h>
#include "../src/cpu.h"

struct machine_t cpu;

static void
setup_cpu(void)
{
    init_machine(&cpu);
}

static TCase*
setup_tcase(char* name)
{
    TCase* tcase = tcase_create(name);
    tcase_add_checked_fixture(tcase, setup_cpu, NULL);
    return tcase;
}

static void
put_opcode(word opcode, address pos)
{
    cpu.mem[pos] = opcode >> 8;
    cpu.mem[pos + 1] = opcode & 0xFF;
}

/* Executing SCD should scroll the screen N pixels down. */
START_TEST(test_scd)
{
    /* Clear the screen, but put an horizontal line on Y = 0. */
    memset(cpu.screen, 0, 2048);
    screen_fill_row(cpu.screen, 0);

    /* Execute SCD 4. */
    cpu.pc = 0x200;
    put_opcode(0x00C4, 0x200);
    step_machine(&cpu);

    /* Test execution. */
    ck_assert_int_eq(0x202, cpu.pc);
    for (int x = 0; x < 64; x++) {
        for (int y = 0; y < 64; y++) {
            if (y == 4) {
                ck_assert_int_ne(0, screen_get_pixel(cpu.screen, x, y));
            } else {
                ck_assert_int_eq(0, screen_get_pixel(cpu.screen, x, y));
            }
        }
    }
}
END_TEST

static TCase*
tcase_scd()
{
    TCase* tcase = setup_tcase("SCD");
    tcase_add_test(tcase, test_scd);
    return tcase;
}

/* Executing SCR should scroll the screen 4 pixels to the right. */
START_TEST(test_scr)
{
    /* Clear the screen and put a vertical line on X = 0; */
    memset(cpu.screen, 0, 2048);
    screen_fill_column(cpu.screen, 0);
    
    /* Execute SCR. */
    cpu.pc = 0x200;
    put_opcode(0x00FB, 0x200);
    step_machine(&cpu);

    /* Test execution. */
    ck_assert_int_eq(0x202, cpu.pc);
    for (int x = 0; x < 64; x++) {
        for (int y = 0; y < 64; y++) {
            if (x == 4) {
                ck_assert_int_ne(0, screen_get_pixel(cpu.screen, x, y));
            } else {
                ck_assert_int_eq(0, screen_get_pixel(cpu.screen, x, y));
            }
        }
    }
}
END_TEST

static TCase*
tcase_scr()
{
    TCase* tcase = setup_tcase("SCR");
    tcase_add_test(tcase, test_scr);
    return tcase;
}

/* Executing SCL should scroll the screen 4 pixels to the left. */
START_TEST(test_scl)
{
    /* Clear the screen and put a vertical line on X = 0; */
    memset(cpu.screen, 0, 2048);
    screen_fill_column(cpu.screen, 4);
    
    /* Execute SCL. */
    cpu.pc = 0x200;
    put_opcode(0x00FC, 0x200);
    step_machine(&cpu);

    /* Test execution. */
    ck_assert_int_eq(0x202, cpu.pc);
    for (int x = 0; x < 64; x++) {
        for (int y = 0; y < 64; y++) {
            if (x == 0) {
                ck_assert_int_ne(0, screen_get_pixel(cpu.screen, x, y));
            } else {
                ck_assert_int_eq(0, screen_get_pixel(cpu.screen, x, y));
            }
        }
    }
}
END_TEST

static TCase*
tcase_scl()
{
    TCase* tcase = setup_tcase("SCL");
    tcase_add_test(tcase, test_scl);
    return tcase;
}

/* Executing EXIT should set the exit flag to true. */
START_TEST(test_exit)
{
    cpu.exit = 0;
    cpu.pc = 0x200;
    put_opcode(0x00FD, 0x200);
    step_machine(&cpu);
    ck_assert_int_eq(0x202, cpu.pc);
    ck_assert_int_ne(0, cpu.exit);
}
END_TEST

static TCase*
tcase_exit()
{
    TCase* tcase = setup_tcase("EXIT");
    tcase_add_test(tcase, test_exit);
    return tcase;
}

/* Executing LOW should disable extended screen mode. */
START_TEST(test_low)
{
    cpu.esm = 1;
    cpu.pc = 0x200;
    put_opcode(0x00FE, 0x200);
    step_machine(&cpu);
    ck_assert_int_eq(0x202, cpu.pc);
    ck_assert_int_eq(0, cpu.esm);
}
END_TEST

static TCase*
tcase_low()
{
    TCase* tcase = setup_tcase("LOW");
    tcase_add_test(tcase, test_low);
    return tcase;
}

/* Executing HIGH should enable extended screen mode. */
START_TEST(test_high)
{
    cpu.esm = 0;
    cpu.pc = 0x200;
    put_opcode(0x00FF, 0x200);
    step_machine(&cpu);
    ck_assert_int_eq(0x202, cpu.pc);
    ck_assert_int_ne(0, cpu.esm);
}
END_TEST

static TCase*
tcase_high()
{
    TCase* tcase = setup_tcase("HIGH");
    tcase_add_test(tcase, test_high);
    return tcase;
}

/* Executing DRAW with extended mode should render a 16x16 sprite. */
START_TEST(test_draw_esm)
{
    /* Set up sprite. */
    for (int i = 0; i < 32; i++) {
        cpu.mem[0x800 + i] = 0xFF;
    }

    /* Set up machine. */
    cpu.esm = 1;
    memcpy(cpu.screen, 0, 8192);
    cpu.pc = 0x200;
    put_opcode(0xD110, 0x200);

    step_machine(&cpu);

    /* Check that the sprite is drawn. */
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 128; x++) {
            if (x < 16 && y < 16) {
                ck_assert_int_ne(0, screen_get_pixel(cpu.screen, x, y));
            } else {
                ck_assert_int_eq(0, screen_get_pixel(cpu.screen, x, y));
            }
        }
    }
    ck_assert_int_eq(0x202, cpu.pc);
}
END_TEST

static TCase*
tcase_draw_esm()
{
    TCase* tcase = setup_tcase("DRW ESM");
    tcase_add_test(tcase, test_draw_esm);
    return tcase;
}
