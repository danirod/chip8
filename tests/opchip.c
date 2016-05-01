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
 * File: tests/opchip.c
 * Description: Unit test related to CHIP-8 Opcodes.
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

/* Should test that upon execution of CLS the screen is cleant. */
START_TEST(test_cls)
{
    memset(cpu.screen, 0x55, 2048);
    put_opcode(0x00E0, 0x00);
    cpu.pc = 0x00;
    step_machine(&cpu);
    for (int i = 0; i < 2048; i++) {
        ck_assert_int_eq(0, cpu.screen[i]);
    }
}
END_TEST

static TCase*
tcase_cls()
{
    TCase* tcase = setup_tcase("CLS");
    tcase_add_test(tcase, test_cls);
    return tcase;
}

START_TEST(test_rts_normal)
{
    cpu.stack[(int) cpu.sp++] = 0x123;
    cpu.stack[(int) cpu.sp++] = 0x234;
    cpu.pc = 0x345;
    ck_assert_int_eq(2, cpu.sp); /* Check I did the math OK. */
    put_opcode(0x00EE, 0x345);
    step_machine(&cpu);
    ck_assert_int_eq(0x234, cpu.pc);
    ck_assert_int_eq(1, cpu.sp);
}
END_TEST

static TCase*
tcase_rts()
{
    TCase* tcase = setup_tcase("RTS");
    tcase_add_test(tcase, test_rts_normal);
    return tcase;
}

START_TEST(test_jmp)
{
    cpu.pc = 0;
    put_opcode(0x1123, 0);
    step_machine(&cpu);
    ck_assert_int_eq(0x123, cpu.pc);
}
END_TEST

static TCase*
tcase_jmp()
{
    TCase* tcase = setup_tcase("JMP");
    tcase_add_test(tcase, test_jmp);
    return tcase;
}

START_TEST(test_jsr_normal)
{
    cpu.pc = 0x55;
    cpu.sp = 0;
    put_opcode(0x2123, 0x55);
    step_machine(&cpu);
    ck_assert_int_eq(1, cpu.sp);
    ck_assert_int_eq(0x123, cpu.pc);
    ck_assert_int_eq(0x57, cpu.stack[0]);
}
END_TEST

static TCase*
tcase_jsr()
{
    TCase* tcase = setup_tcase("JSR");
    tcase_add_test(tcase, test_jsr_normal);
    return tcase;
}

START_TEST(test_se_eq)
{
    cpu.v[4] = 0x55;
    cpu.pc = 0x00;
    put_opcode(0x3455, 0x00);
    step_machine(&cpu);
    ck_assert_int_eq(4, cpu.pc);
}
END_TEST

START_TEST(test_se_ne)
{
    cpu.v[4] = 0x54;
    cpu.pc = 0x00;
    put_opcode(0x3455, 0x00);
    step_machine(&cpu);
    ck_assert_int_eq(2, cpu.pc);
}
END_TEST

static TCase*
tcase_se()
{
    TCase* tcase = setup_tcase("SE");
    tcase_add_test(tcase, test_se_eq);
    tcase_add_test(tcase, test_se_ne);
    return tcase;
}

START_TEST(test_sne_eq)
{
    cpu.v[4] = 0x55;
    cpu.pc = 0x00;
    put_opcode(0x4455, 0x00);
    step_machine(&cpu);
    ck_assert_int_eq(2, cpu.pc);
}
END_TEST

START_TEST(test_sne_ne)
{
    cpu.v[4] = 0x54;
    cpu.pc = 0x00;
    put_opcode(0x4455, 0x00);
    step_machine(&cpu);
    ck_assert_int_eq(4, cpu.pc);
}
END_TEST

static TCase*
tcase_sne()
{
    TCase* tcase = setup_tcase("SNE");
    tcase_add_test(tcase, test_sne_eq);
    tcase_add_test(tcase, test_sne_ne);
    return tcase;
}

START_TEST(test_sexy_eq)
{
    cpu.v[4] = 0x55;
    cpu.v[5] = 0x55;
    cpu.pc = 0x00;
    put_opcode(0x5450, 0x00);
    step_machine(&cpu);
    ck_assert_int_eq(4, cpu.pc);
}
END_TEST

START_TEST(test_sexy_ne)
{
    cpu.v[4] = 0x54;
    cpu.v[5] = 0x55;
    cpu.pc = 0x00;
    put_opcode(0x5450, 0x00);
    step_machine(&cpu);
    ck_assert_int_eq(2, cpu.pc);
}
END_TEST

static TCase*
tcase_sexy()
{
    TCase* tcase = setup_tcase("SEXY");
    tcase_add_test(tcase, test_sexy_eq);
    tcase_add_test(tcase, test_sexy_ne);
    return tcase;
}

START_TEST(test_ld)
{
    cpu.v[5] = 0x12;
    cpu.pc = 0;
    put_opcode(0x6534, 0);
    step_machine(&cpu);
    ck_assert_int_eq(0x34, cpu.v[5]);
}
END_TEST

static TCase*
tcase_ld()
{
    TCase* tcase = setup_tcase("LD");
    tcase_add_test(tcase, test_ld);
    return tcase;
}

START_TEST(test_add)
{
    cpu.v[5] = 0x12;
    cpu.pc = 0;
    put_opcode(0x7534, 0);
    step_machine(&cpu);
    ck_assert_int_eq(0x46, cpu.v[5]);
}
END_TEST

static TCase*
tcase_add()
{
    TCase* tcase = setup_tcase("ADD");
    tcase_add_test(tcase, test_add);
    return tcase;
}

START_TEST(test_ldxy)
{
    cpu.v[4] = 0x33;
    cpu.v[5] = 0x55;
    cpu.pc = 0;
    put_opcode(0x8450, 0);
    step_machine(&cpu);
    ck_assert_int_eq(0x55, cpu.v[4]);
}
END_TEST

static TCase*
tcase_ldxy()
{
    TCase *tcase = setup_tcase("LDXY");
    tcase_add_test(tcase, test_ldxy);
    return tcase;
}

START_TEST(test_orxy)
{
    cpu.v[4] = 0x40;
    cpu.v[5] = 0x04;
    cpu.pc = 0;
    put_opcode(0x8451, 0);
    step_machine(&cpu);
    ck_assert_int_eq(0x44, cpu.v[4]);
}
END_TEST

static TCase*
tcase_orxy()
{
    TCase* tcase = setup_tcase("ORXY");
    tcase_add_test(tcase, test_orxy);
    return tcase;
}

START_TEST(test_andxy)
{
    cpu.v[4] = 0x40;
    cpu.v[5] = 0x04;
    cpu.pc = 0;
    put_opcode(0x8452, 0);
    step_machine(&cpu);
    ck_assert_int_eq(0x00, cpu.v[4]);
}
END_TEST

static TCase*
tcase_andxy()
{
    TCase* tcase = setup_tcase("ANDXY");
    tcase_add_test(tcase, test_andxy);
    return tcase;
}

START_TEST(test_xorxy)
{
    cpu.v[4] = 0x55;
    cpu.v[5] = 0xAA;
    cpu.pc = 0;
    put_opcode(0x8453, 0);
    step_machine(&cpu);
    ck_assert_int_eq(0xFF, cpu.v[4]);
}
END_TEST

static TCase*
tcase_xorxy()
{
    TCase* tcase = setup_tcase("XORXY");
    tcase_add_test(tcase, test_xorxy);
    return tcase;
}

START_TEST(test_addxy_nocarry)
{
    cpu.v[4] = 0x12;
    cpu.v[5] = 0x34;
    cpu.pc = 0;
    put_opcode(0x8454, 0);
    step_machine(&cpu);
    ck_assert_int_eq(0x46, cpu.v[4]);
    ck_assert_int_eq(0, cpu.v[0xf]);
}
END_TEST

START_TEST(test_addxy_carry)
{
    cpu.v[4] = 0xF0;
    cpu.v[5] = 0xF0;
    cpu.pc = 0;
    put_opcode(0x8454, 0);
    step_machine(&cpu);
    ck_assert_int_eq(0xE0, cpu.v[4]);
    ck_assert_int_eq(1, cpu.v[0xf]);
}
END_TEST

static TCase*
tcase_addxy()
{
    TCase* tcase = setup_tcase("ADDXY");
    tcase_add_test(tcase, test_addxy_nocarry);
    tcase_add_test(tcase, test_addxy_carry);
    return tcase;
}

START_TEST(test_subxy_noborrow)
{
    cpu.v[4] = 0x46;
    cpu.v[5] = 0x34;
    cpu.pc = 0;
    put_opcode(0x8455, 0);
    step_machine(&cpu);
    ck_assert_int_eq(0x12, cpu.v[4]);
    ck_assert_int_eq(1, cpu.v[0xf]);
}
END_TEST

START_TEST(test_subxy_borrow)
{
    cpu.v[4] = 0x30;
    cpu.v[5] = 0x40;
    cpu.pc = 0;
    put_opcode(0x8455, 0);
    step_machine(&cpu);
    ck_assert_int_eq(0xF0, cpu.v[4]);
    ck_assert_int_eq(0, cpu.v[0xf]);
}
END_TEST

static TCase*
tcase_subxy()
{
    TCase* tcase = setup_tcase("SUBXY");
    tcase_add_test(tcase, test_subxy_noborrow);
    tcase_add_test(tcase, test_subxy_borrow);
    return tcase;
}

START_TEST(test_shr_1)
{
    cpu.v[4] = 0x45;
    put_opcode(0x8406, 0);
    cpu.pc = 0x00;
    step_machine(&cpu);
    ck_assert_int_eq(0x22, cpu.v[4]);
    ck_assert_int_eq(1, cpu.v[0xf]);
}
END_TEST

START_TEST(test_shr_0)
{
    cpu.v[4] = 0x44;
    put_opcode(0x8406, 0);
    cpu.pc = 0x00;
    step_machine(&cpu);
    ck_assert_int_eq(0x22, cpu.v[4]);
    ck_assert_int_eq(0, cpu.v[0xf]);
}
END_TEST

static TCase*
tcase_shr()
{
    TCase* tcase = setup_tcase("SHR");
    tcase_add_test(tcase, test_shr_0);
    tcase_add_test(tcase, test_shr_1);
    return tcase;
}

START_TEST(test_subnxy_noborrow)
{
    cpu.v[4] = 0x34;
    cpu.v[5] = 0x46;
    cpu.pc = 0;
    put_opcode(0x8457, 0);
    step_machine(&cpu);
    ck_assert_int_eq(0x12, cpu.v[4]);
    ck_assert_int_eq(1, cpu.v[0xf]);
}
END_TEST

START_TEST(test_subnxy_borrow)
{
    cpu.v[4] = 0x40;
    cpu.v[5] = 0x30;
    cpu.pc = 0;
    put_opcode(0x8457, 0);
    step_machine(&cpu);
    ck_assert_int_eq(0xF0, cpu.v[4]);
    ck_assert_int_eq(0, cpu.v[0xf]);
}
END_TEST

static TCase*
tcase_subnxy()
{
    TCase* tcase = setup_tcase("SUBN");
    tcase_add_test(tcase, test_subnxy_noborrow);
    tcase_add_test(tcase, test_subnxy_borrow);
    return tcase;
}

START_TEST(test_shl_0)
{
    cpu.v[4] = 0x08;
    cpu.pc = 0;
    put_opcode(0x840E, 0);
    step_machine(&cpu);
    ck_assert_int_eq(0x10, cpu.v[4]);
    ck_assert_int_eq(0, cpu.v[0xf]);
}
END_TEST

START_TEST(test_shl_1)
{
    cpu.v[4] = 0xC8;
    cpu.pc = 0;
    put_opcode(0x840E, 0);
    step_machine(&cpu);
    ck_assert_int_eq(0x90, cpu.v[4]);
    ck_assert_int_eq(1, cpu.v[0xf]);
}
END_TEST

static TCase*
tcase_shl()
{
    TCase* tcase = setup_tcase("SHL");
    tcase_add_test(tcase, test_shl_0);
    tcase_add_test(tcase, test_shl_1);
    return tcase;
}

START_TEST(test_snexy_eq)
{
    cpu.v[4] = 0x55;
    cpu.v[5] = 0x55;
    cpu.pc = 0;
    put_opcode(0x9450, 0);
    step_machine(&cpu);
    ck_assert_int_eq(2, cpu.pc);
}
END_TEST

START_TEST(test_snexy_ne)
{
    cpu.v[4] = 0x55;
    cpu.v[5] = 0x56;
    cpu.pc = 0;
    put_opcode(0x9450, 0);
    step_machine(&cpu);
    ck_assert_int_eq(4, cpu.pc);
}
END_TEST

static TCase*
tcase_snexy()
{
    TCase* tcase = setup_tcase("SNEXY");
    tcase_add_test(tcase, test_snexy_eq);
    tcase_add_test(tcase, test_snexy_ne);
    return tcase;
}

START_TEST(test_ldi)
{
    cpu.i = 0;
    cpu.pc = 0;
    put_opcode(0xA123, 0);
    step_machine(&cpu);
    ck_assert_int_eq(0x123, cpu.i);
}
END_TEST

static TCase*
tcase_ldi()
{
    TCase* tcase = setup_tcase("LDI");
    tcase_add_test(tcase, test_ldi);
    return tcase;
}

START_TEST(test_jp)
{
    cpu.v[0] = 0x55;
    cpu.pc = 0;
    put_opcode(0xB123, 0);
    step_machine(&cpu);
    ck_assert_int_eq(0x178, cpu.pc);
}
END_TEST

static TCase*
tcase_jp()
{
    TCase* tcase = setup_tcase("JP");
    tcase_add_test(tcase, test_jp);
    return tcase;
}

static int
mock_poller(char key)
{
    return key == 2;
}

START_TEST(test_skp)
{
    cpu.keydown = &mock_poller;
    put_opcode(0xE09E, 0);
    for (char key = 0; key < 16; key++) {
        cpu.pc = 0;
        cpu.v[0] = key;
        step_machine(&cpu);
        if (key == 2) {
            ck_assert_int_eq(4, cpu.pc);
        } else {
            ck_assert_int_eq(2, cpu.pc);
        }
    }
}
END_TEST

static TCase*
tcase_skp()
{
    TCase* tcase = setup_tcase("SKP");
    tcase_add_test(tcase, test_skp);
    return tcase;
}

START_TEST(test_sknp)
{
    cpu.keydown = &mock_poller;
    put_opcode(0xE0A1, 0);
    for (char key = 0; key < 16; key++) {
        cpu.pc = 0;
        cpu.v[0] = key;
        step_machine(&cpu);
        if (key != 2) {
            ck_assert_int_eq(4, cpu.pc);
        } else {
            ck_assert_int_eq(2, cpu.pc);
        }
    }
}
END_TEST

static TCase*
tcase_sknp()
{
    TCase* tcase = setup_tcase("SKNP");
    tcase_add_test(tcase, test_sknp);
    return tcase;
}

START_TEST(test_lddt_in)
{
    cpu.dt = 0x55;
    cpu.v[0] = 0;
    put_opcode(0xF007, 0);
    cpu.pc = 0x00;
    step_machine(&cpu);
    ck_assert_int_eq(0x55, cpu.v[0]);
}
END_TEST

START_TEST(test_lddt_out)
{
    put_opcode(0xF015, 0);
    cpu.pc = 0x00;
    cpu.v[0] = 0x55;
    cpu.dt = 0;
    step_machine(&cpu);
    ck_assert_int_eq(0x55, cpu.dt);
}
END_TEST

static TCase*
tcase_lddt()
{
    TCase* tcase = setup_tcase("LDDT");
    tcase_add_test(tcase, test_lddt_out);
    tcase_add_test(tcase, test_lddt_in);
    return tcase;
}

START_TEST(test_ldk)
{
    cpu.keydown = &mock_poller;
    cpu.v[0] = 0xFF;
    put_opcode(0xF00A, 0);
    cpu.pc = 0x00;
    step_machine(&cpu);
    /* Since this opcode should wait, we have to iterate a new step. */
    step_machine(&cpu);
    ck_assert_int_eq(2, cpu.v[0]);
}
END_TEST

static TCase*
tcase_ldk()
{
    TCase* tcase = setup_tcase("LDK");
    tcase_add_test(tcase, test_ldk);
    return tcase;
}

START_TEST(test_ldst)
{
    put_opcode(0xF018, 0);
    cpu.v[0] = 0x55;
    cpu.st = 0;
    cpu.pc = 0x00;
    step_machine(&cpu);
    ck_assert_int_eq(0x55, cpu.st);
}
END_TEST

static TCase*
tcase_ldst()
{
    TCase* tcase = setup_tcase("LDST");
    tcase_add_test(tcase, test_ldst);
    return tcase;
}

START_TEST(test_addi)
{
    put_opcode(0xF01E, 0);
    cpu.v[0] = 0x30;
    cpu.i = 0x400;
    cpu.pc = 0x00;
    step_machine(&cpu);
    ck_assert_int_eq(0x430, cpu.i);
}
END_TEST

static TCase*
tcase_addi()
{
    TCase* tcase = setup_tcase("ADDI");
    tcase_add_test(tcase, test_addi);
    return tcase;
}

START_TEST(test_ldf)
{
    cpu.v[0] = 3;
    cpu.i = 0;
    cpu.pc = 0x00;
    put_opcode(0xF029, 0);
    step_machine(&cpu);
    ck_assert_int_eq(0x50 + 15, cpu.i);
}
END_TEST

static TCase*
tcase_ldf()
{
    TCase* tcase = setup_tcase("LDF");
    tcase_add_test(tcase, test_ldf);
    return tcase;
}

START_TEST(test_ldb)
{
    cpu.v[0] = 123;
    cpu.i = 0x400;
    cpu.pc = 0x00;
    put_opcode(0xF033, 0);
    step_machine(&cpu);
    ck_assert_int_eq(1, cpu.mem[0x400]);
    ck_assert_int_eq(2, cpu.mem[0x401]);
    ck_assert_int_eq(3, cpu.mem[0x402]);
}
END_TEST

static TCase*
tcase_ldb()
{
    TCase* tcase = setup_tcase("LDB");
    tcase_add_test(tcase, test_ldb);
    return tcase;
}

START_TEST(test_ldix_out)
{
    for (int i = 0; i < 16; i++) {
        cpu.v[i] = 0x80 + i;
        cpu.mem[0x400 + i] = 0xFF;
    }
    put_opcode(0xFF55, 0);
    cpu.i = 0x400;
    cpu.pc = 0x00;
    step_machine(&cpu);
    for (int i = 0; i < 16; i++) {
        ck_assert_int_eq(0x80 + i, cpu.mem[0x400 + i]);
    }
}
END_TEST

START_TEST(test_ldix_in)
{
    for (int i = 0; i < 16; i++) {
        cpu.mem[0x400 + i] = 0x80 + i;
        cpu.v[i] = 0xFF;
    }
    put_opcode(0xFF65, 0);
    cpu.i = 0x400;
    cpu.pc = 0x00;
    step_machine(&cpu);
    for (int i = 0; i < 16; i++) {
        ck_assert_int_eq(0x80 + i, cpu.v[i]);
    }
}
END_TEST

static TCase*
tcase_ldix()
{
    TCase* tcase = setup_tcase("LDIX");
    tcase_add_test(tcase, test_ldix_in);
    tcase_add_test(tcase, test_ldix_out);
    return tcase;
}

Suite*
create_chip8_opcodes_suite()
{
    Suite* suite = suite_create("CHIP-8 Opcodes");
    suite_add_tcase(suite, tcase_cls());
    suite_add_tcase(suite, tcase_rts());
    suite_add_tcase(suite, tcase_jmp());
    suite_add_tcase(suite, tcase_jsr());
    suite_add_tcase(suite, tcase_se());
    suite_add_tcase(suite, tcase_sne());
    suite_add_tcase(suite, tcase_sexy());
    suite_add_tcase(suite, tcase_ld());
    suite_add_tcase(suite, tcase_add());
    suite_add_tcase(suite, tcase_ldxy());
    suite_add_tcase(suite, tcase_orxy());
    suite_add_tcase(suite, tcase_andxy());
    suite_add_tcase(suite, tcase_xorxy());
    suite_add_tcase(suite, tcase_addxy());
    suite_add_tcase(suite, tcase_subxy());
    suite_add_tcase(suite, tcase_shr());
    suite_add_tcase(suite, tcase_subnxy());
    suite_add_tcase(suite, tcase_shl());
    suite_add_tcase(suite, tcase_snexy());
    suite_add_tcase(suite, tcase_ldi());
    suite_add_tcase(suite, tcase_jp());
    suite_add_tcase(suite, tcase_skp());
    suite_add_tcase(suite, tcase_sknp());
    suite_add_tcase(suite, tcase_lddt());
    suite_add_tcase(suite, tcase_ldk());
    suite_add_tcase(suite, tcase_ldst());
    suite_add_tcase(suite, tcase_addi());
    suite_add_tcase(suite, tcase_ldf());
    suite_add_tcase(suite, tcase_ldb());
    suite_add_tcase(suite, tcase_ldix());
    return suite;
}
