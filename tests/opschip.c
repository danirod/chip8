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

static void
put_opcode(word opcode, address pos)
{
    cpu.mem[pos] = opcode >> 8;
    cpu.mem[pos + 1] = opcode & 0xFF;
}

/* Executing SCD should scroll the screen N pixels down. */
START_TEST(test_scd_esm_off)
{
    /* Clear the screen, but put an horizontal line on Y = 0. */
    cpu.esm = 0;
    memset(cpu.screen, 0, 2048);
    screen_fill_row(&cpu, 0);

    /* Execute SCD 4. */
    cpu.pc = 0x200;
    put_opcode(0x00C4, 0x200);
    step_machine(&cpu);

    /* Test execution. */
    ck_assert_int_eq(0x202, cpu.pc);
    for (int row = 0; row < 32; row++) {
        for (int col = 0; col < 64; col++) {
            /* White row on Y = 0 should still be there too!. */
            if (row == 0 || row == 4) {
                ck_assert_int_ne(0, screen_get_pixel(&cpu, row, col));
            } else {
                ck_assert_int_eq(0, screen_get_pixel(&cpu, row, col));
            }
        }
    }
}
END_TEST

START_TEST(test_scd_esm_on)
{
    /* Clear the screen, put an horizontal line on Y = 0. */
    cpu.esm = 1;
    memset(cpu.screen, 0, 8192);
    screen_fill_row(&cpu, 0);

    /* Execute SCD 4. */
    cpu.pc = 0x200;
    put_opcode(0x00C4, 0x200);
    step_machine(&cpu);

    /* Test execution. */
    ck_assert_int_eq(0x202, cpu.pc);
    for (int row = 0; row < 64; row++) {
        for (int col = 0; col < 128; col++) {
            /* Again, there should still be a line on Y = 0!. */
            if (row == 0 || row == 4) {
                ck_assert_int_ne(0, screen_get_pixel(&cpu, row, col));
            } else {
                ck_assert_int_eq(0, screen_get_pixel(&cpu, row, col));
            }
        }
    }
}
END_TEST

static TCase*
tcase_scd()
{
    TCase* tcase = setup_tcase("SCD");
    tcase_add_test(tcase, test_scd_esm_off);
    tcase_add_test(tcase, test_scd_esm_on);
    return tcase;
}

/* Executing SCR should scroll the screen 4 pixels to the right. */
START_TEST(test_scr_esm_off)
{
    /* Clear the screen and put a vertical line on X = 0. */
    cpu.esm = 0;
    memset(cpu.screen, 0, 2048);
    screen_fill_column(&cpu, 0);
    
    /* Execute SCR. */
    cpu.pc = 0x200;
    put_opcode(0x00FB, 0x200);
    step_machine(&cpu);

    /* Test execution. */
    ck_assert_int_eq(0x202, cpu.pc);
    for (int row = 0; row < 32; row++) {
        for (int col = 0; col < 64; col++) {
            /* Since we are scrolling right, line at X = 0 should be there. */
            if (col == 0 || col == 4) {
                ck_assert_int_ne(0, screen_get_pixel(&cpu, row, col));
            } else {
                ck_assert_int_eq(0, screen_get_pixel(&cpu, row, col));
            }
        }
    }
}
END_TEST

START_TEST(test_scr_esm_on)
{
    /* Clear screen, put vertical line on X = 0. */
    cpu.esm = 1;
    memset(cpu.screen, 0, 8192);
    screen_fill_column(&cpu, 0);

    /* Execute SCR. */
    cpu.pc = 0x200;
    put_opcode(0x00FB, 0x200);
    step_machine(&cpu);

    /* Test execution. */
    ck_assert_int_eq(0x202, cpu.pc);
    for (int row = 0; row < 64; row++) {
        for (int col = 0; col < 128; col++) {
            /* Since we scroll right, line at X = 0 should be there too. */
            if (col == 0 || col == 4) {
                ck_assert_int_ne(0, screen_get_pixel(&cpu, row, col));
            } else {
                ck_assert_int_eq(0, screen_get_pixel(&cpu, row, col));
            }
        }
    }
}
END_TEST

static TCase*
tcase_scr()
{
    TCase* tcase = setup_tcase("SCR");
    tcase_add_test(tcase, test_scr_esm_off);
    tcase_add_test(tcase, test_scr_esm_on);
    return tcase;
}

/* Executing SCL should scroll the screen 4 pixels to the left. */
START_TEST(test_scl_esm_off)
{
    /* Clear the screen and put a vertical line on X = 0. */
    memset(cpu.screen, 0, 2048);
    screen_fill_column(&cpu, 4);
    
    /* Execute SCL. */
    cpu.pc = 0x200;
    put_opcode(0x00FC, 0x200);
    step_machine(&cpu);

    /* Test execution. */
    ck_assert_int_eq(0x202, cpu.pc);
    for (int row = 0; row < 32; row++) {
        for (int col = 0; col < 64; col++) {
            /* There is no line at X = 4 because is overwritten by X = 8. */
            if (col == 0) {
                ck_assert_int_ne(0, screen_get_pixel(&cpu, row, col));
            } else {
                ck_assert_int_eq(0, screen_get_pixel(&cpu, row, col));
            }
        }
    }
}
END_TEST

START_TEST(test_scl_esm_on)
{
    /* Clear thes creen and put a vertical line on X = 4. */
    cpu.esm = 1;
    memset(cpu.screen, 0, 8192);
    screen_fill_column(&cpu, 4);

    /* Execute SCL. */
    cpu.pc = 0x200;
    put_opcode(0x00FC, 0x200);
    step_machine(&cpu);

    /* Test execution. */
    ck_assert_int_eq(0x202, cpu.pc);
    for (int row = 0; row < 64; row++) {
        for (int col = 0; col < 128; col++) {
            /* There is no line at X = 4 because is overwritten by X = 8. */
            if (col == 0) {
                ck_assert_int_ne(0, screen_get_pixel(&cpu, row, col));
            } else {
                ck_assert_int_eq(0, screen_get_pixel(&cpu, row, col));
            }
        }
    }
}
END_TEST

static TCase*
tcase_scl()
{
    TCase* tcase = setup_tcase("SCL");
    tcase_add_test(tcase, test_scl_esm_off);
    tcase_add_test(tcase, test_scl_esm_on);
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
    memset(cpu.screen, 0, 8192);
    cpu.i = 0x800;
    put_opcode(0xD110, 0x200);
    step_machine(&cpu);

    /* Check that the sprite is drawn. */
    ck_assert_int_eq(0x202, cpu.pc);
    for (int row = 0; row < 64; row++) {
        for (int col = 0; col < 128; col++) {
            if (row < 16 && col < 16) {
                ck_assert_int_ne(0, screen_get_pixel(&cpu, row, col));
            } else {
                ck_assert_int_eq(0, screen_get_pixel(&cpu, row, col));
            }
        }
    }
}
END_TEST

static TCase*
tcase_draw_esm()
{
    TCase* tcase = setup_tcase("DRW ESM");
    tcase_add_test(tcase, test_draw_esm);
    return tcase;
}

START_TEST(test_ld_hf)
{
    cpu.esm = 1;
    put_opcode(0xF030, 0x200);
    for (int r = 0; r < 16; r++) {
        cpu.v[0] = r;
        cpu.pc = 0x200;
        step_machine(&cpu);
        ck_assert_int_eq(0x8200 + r * 10, cpu.i);
    }
}
END_TEST

static TCase*
tcase_ld_hf()
{
    TCase* tcase = setup_tcase("LD HF");
    tcase_add_test(tcase, test_ld_hf);
    return tcase;
}

START_TEST(test_ld_r_v)
{
    for (int rg = 0; rg < 8; rg++) {
        cpu.v[rg] = rg * 3;
        cpu.r[rg] = 0xFF;
    }
    cpu.pc = 0x200;
    put_opcode(0xF775, 0x200);
    step_machine(&cpu);
    for (int rg = 0; rg < 8; rg++) {
        ck_assert_int_eq(rg * 3, cpu.r[rg]);
    }
}
END_TEST

START_TEST(test_ld_r_v_partial)
{
    for (int rg = 0; rg < 8; rg++) {
        cpu.v[rg] = rg * 3;
        cpu.r[rg] = 0xFF;
    }
    cpu.pc = 0x200;
    put_opcode(0xF475, 0x200);
    step_machine(&cpu);
    for (int rg = 0; rg < 8; rg++) {
        if (rg <= 4) {
            ck_assert_int_eq(rg * 3, cpu.r[rg]);
        } else {
            ck_assert_int_eq(0xFF, cpu.r[rg]);
        }
    }
}
END_TEST

static TCase*
tcase_ld_r_v()
{
    TCase* tcase = setup_tcase("LD R, V");
    tcase_add_test(tcase, test_ld_r_v);
    tcase_add_test(tcase, test_ld_r_v_partial);
    return tcase;
}

START_TEST(test_ld_v_r)
{
    for (int rg = 0; rg < 8; rg++) {
        cpu.r[rg] = rg * 3;
        cpu.v[rg] = 0xFF;
    }
    cpu.pc = 0x200;
    put_opcode(0xF785, 0x200);
    step_machine(&cpu);
    for (int rg = 0; rg < 8; rg++) {
        ck_assert_int_eq(rg * 3, cpu.v[rg]);
    }
}
END_TEST

START_TEST(test_ld_v_r_partial)
{
    for (int rg = 0; rg < 8; rg++) {
        cpu.r[rg] = rg * 3;
        cpu.v[rg] = 0xFF;
    }
    cpu.pc = 0x200;
    put_opcode(0xF485, 0x200);
    step_machine(&cpu);
    for (int rg = 0; rg < 8; rg++) {
        if (rg <= 4) {
            ck_assert_int_eq(rg * 3, cpu.v[rg]);
        } else {
            ck_assert_int_eq(0xFF, cpu.v[rg]);
        }
    }
}
END_TEST

static TCase*
tcase_ld_v_r()
{
    TCase* tcase = setup_tcase("LD V, R");
    tcase_add_test(tcase, test_ld_v_r);
    tcase_add_test(tcase, test_ld_v_r_partial);
    return tcase;
}

Suite*
create_superchip_opcodes_suite()
{
    Suite* suite = suite_create("SCHIP Opcodes");
    suite_add_tcase(suite, tcase_scd());
    suite_add_tcase(suite, tcase_scr());
    suite_add_tcase(suite, tcase_scl());
    suite_add_tcase(suite, tcase_exit());
    suite_add_tcase(suite, tcase_high());
    suite_add_tcase(suite, tcase_low());
    suite_add_tcase(suite, tcase_draw_esm());
    suite_add_tcase(suite, tcase_ld_hf());
    suite_add_tcase(suite, tcase_ld_r_v());
    suite_add_tcase(suite, tcase_ld_v_r());
    return suite;
}

