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
 * File: tests/screen.c
 * Description: Unit test related to screen management.
 */

#include <check.h>
#include <stdint.h>
#include <lib8/cpu.h>

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

START_TEST(test_screen_fill_column)
{
    cpu.esm = 0;
    memset(cpu.screen, 0, sizeof (cpu.screen));
    screen_fill_column(&cpu, 4);
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (x == 4) {
                ck_assert_int_ne(0, cpu.screen[64 * y + x]);
            } else {
                ck_assert_int_eq(0, cpu.screen[64 * y + x]);
            }
        }
    }
}
END_TEST

START_TEST(test_screen_clear_column)
{
    cpu.esm = 0;
    memset(cpu.screen, 1, sizeof (cpu.screen));
    screen_clear_column(&cpu, 8);
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (x == 8) {
                ck_assert_int_eq(0, cpu.screen[64 * y + x]);
            } else {
                ck_assert_int_ne(0, cpu.screen[64 * y + x]);
            }
        }
    }
}
END_TEST

START_TEST(test_screen_fill_row)
{
    cpu.esm = 0;
    memset(cpu.screen, 0, sizeof(cpu.screen));
    screen_fill_row(&cpu, 4);
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (y == 4) {
                ck_assert_int_ne(0, cpu.screen[64 * y + x]);
            } else {
                ck_assert_int_eq(0, cpu.screen[64 * y + x]);
            }
        }
    }
}
END_TEST

START_TEST(test_screen_clear_row)
{
    cpu.esm = 0;
    memset(cpu.screen, 1, sizeof(cpu.screen));
    screen_clear_row(&cpu, 6);
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (y == 6) {
                ck_assert_int_eq(0, cpu.screen[64 * y + x]);
            } else {
                ck_assert_int_ne(0, cpu.screen[64 * y + x]);
            }
        }
    }
}
END_TEST

START_TEST(test_screen_get_pixel)
{
    cpu.esm = 0;
    memset(cpu.screen, 0, sizeof (cpu.screen));
    cpu.screen[64 * 10 + 10] = 1;
    cpu.screen[64 * 20 + 20] = 1;
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (x == 10 && y == 10) {
                ck_assert_int_ne(0, screen_get_pixel(&cpu, x, y));
            } else if (x == 20 && y == 20) {
                ck_assert_int_ne(0, screen_get_pixel(&cpu, x, y));
            } else {
                ck_assert_int_eq(0, screen_get_pixel(&cpu, x, y));
            }
        }
    }
}
END_TEST

START_TEST(test_screen_set_pixel)
{
    cpu.esm = 0;
    memset(cpu.screen, 0, sizeof(cpu.screen));
    screen_set_pixel(&cpu, 10, 10);
    screen_set_pixel(&cpu, 20, 20);
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (x == 10 && y == 10) {
                ck_assert_int_ne(0, cpu.screen[64 * y + x]);
            } else if (x == 20 && y == 20) {
                ck_assert_int_ne(0, cpu.screen[64 * y + x]);
            } else {
                ck_assert_int_eq(0, cpu.screen[64 * y + x]);
            }
        }
    }
}
END_TEST

START_TEST(test_screen_clear_pixel)
{
    cpu.esm = 0;
    memset(cpu.screen, 0, sizeof (cpu.screen));
    cpu.screen[64 * 10 + 10] = 1;
    cpu.screen[64 * 20 + 20] = 1;
    screen_clear_pixel(&cpu, 10, 10);
    screen_clear_pixel(&cpu, 20, 20);
    ck_assert_int_eq(0, cpu.screen[64 * 10 + 10]);
    ck_assert_int_eq(0, cpu.screen[64 * 20 + 20]);
}
END_TEST

static TCase*
tcase_screen_fill_column()
{
    TCase* tcase = setup_tcase("screen_fill_column()");
    tcase_add_test(tcase, test_screen_fill_column);
    return tcase;
}

static TCase*
tcase_screen_clear_column()
{
    TCase* tcase = setup_tcase("screen_clear_column()");
    tcase_add_test(tcase, test_screen_clear_column);
    return tcase;
}

static TCase*
tcase_screen_fill_row()
{
    TCase* tcase = setup_tcase("screen_fill_row()");
    tcase_add_test(tcase, test_screen_fill_row);
    return tcase;
}

static TCase*
tcase_screen_clear_row()
{
    TCase* tcase = setup_tcase("screen_clear_row()");
    tcase_add_test(tcase, test_screen_clear_row);
    return tcase;
}

static TCase*
tcase_screen_get_pixel()
{
    TCase* tcase = setup_tcase("screen_get_pixel()");
    tcase_add_test(tcase, test_screen_get_pixel);
    return tcase;
}

static TCase*
tcase_screen_set_pixel()
{
    TCase* tcase = setup_tcase("screen_set_pixel()");
    tcase_add_test(tcase, test_screen_set_pixel);
    return tcase;
}

static TCase*
tcase_screen_clear_pixel()
{
    TCase* tcase = setup_tcase("screen_clear_pixel()");
    tcase_add_test(tcase, test_screen_clear_pixel);
    return tcase;
}

Suite*
create_screen_suite()
{
    Suite* suite = suite_create("Screen management");
    suite_add_tcase(suite, tcase_screen_fill_column());
    suite_add_tcase(suite, tcase_screen_clear_column());
    suite_add_tcase(suite, tcase_screen_fill_row());
    suite_add_tcase(suite, tcase_screen_clear_row());
    suite_add_tcase(suite, tcase_screen_get_pixel());
    suite_add_tcase(suite, tcase_screen_set_pixel());
    suite_add_tcase(suite, tcase_screen_clear_pixel());
    return suite;
}
