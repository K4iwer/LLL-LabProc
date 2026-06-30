/*
 * Calculadora equivalente em RISC-V RV32IM (mesma ISA base do ESP32-C3).
 *
 * Entradas de calc_riscv:
 *   a0 = operando A
 *   a1 = operando B
 *   a2 = operacao: 0 soma, 1 subtracao, 2 multiplicacao,
 *                  3 fatorial, 4 divisao
 *
 * Saidas:
 *   a0 = resultado
 *   a1 = status: 0 sucesso, 1 divisao por zero, 2 operacao invalida,
 *                3 fatorial negativo, 4 overflow
 *
 * O bloco _start executa os mesmos casos do arquivo ARM.asm. Inspecione
 * riscv_results e riscv_status quando o simulador chegar a riscv_stop.
 */

    .equ OP_ADD, 0
    .equ OP_SUB, 1
    .equ OP_MUL, 2
    .equ OP_FAT, 3
    .equ OP_DIV, 4
    .equ RISCV_TEST_COUNT, 8

    .section .text
    .globl _start
    .globl main
    .globl calc_riscv

main:
_start:
    la      s0, riscv_test_cases
    la      s1, riscv_results
    la      s2, riscv_status
    li      s3, RISCV_TEST_COUNT

riscv_test_loop:
    lw      a0, 0(s0)
    lw      a1, 4(s0)
    lw      a2, 8(s0)
    addi    s0, s0, 12
    jal     ra, calc_riscv
    sw      a0, 0(s1)
    sw      a1, 0(s2)
    addi    s1, s1, 4
    addi    s2, s2, 4
    addi    s3, s3, -1
    bnez    s3, riscv_test_loop

riscv_stop:
    j       riscv_stop

calc_riscv:
    li      t0, OP_ADD
    beq     a2, t0, riscv_add
    li      t0, OP_SUB
    beq     a2, t0, riscv_sub
    li      t0, OP_MUL
    beq     a2, t0, riscv_mul
    li      t0, OP_FAT
    beq     a2, t0, riscv_factorial
    li      t0, OP_DIV
    beq     a2, t0, riscv_div
    j       riscv_invalid_operation

riscv_add:
    add     t0, a0, a1
    /* Overflow assinado: operandos com mesmo sinal e resultado diferente. */
    xor     t1, a0, a1
    xor     t2, a0, t0
    not     t1, t1
    and     t1, t1, t2
    bltz    t1, riscv_overflow
    mv      a0, t0
    j       riscv_success

riscv_sub:
    sub     t0, a0, a1
    /* Overflow assinado: operandos diferentes e resultado difere de A. */
    xor     t1, a0, a1
    xor     t2, a0, t0
    and     t1, t1, t2
    bltz    t1, riscv_overflow
    mv      a0, t0
    j       riscv_success

riscv_mul:
    mul     t0, a0, a1
    mulh    t1, a0, a1
    srai    t2, t0, 31
    bne     t1, t2, riscv_overflow
    mv      a0, t0
    j       riscv_success

riscv_factorial:
    bltz    a0, riscv_negative_factorial
    li      t0, 12
    bgt     a0, t0, riscv_overflow

    mv      t0, a0
    li      a0, 1
    li      t1, 1
    ble     t0, t1, riscv_success

riscv_factorial_loop:
    mul     a0, a0, t0
    addi    t0, t0, -1
    li      t1, 1
    bgt     t0, t1, riscv_factorial_loop
    j       riscv_success

riscv_div:
    beqz    a1, riscv_division_by_zero

    /* INT32_MIN / -1 satura em RISC-V; tratamos como overflow explicito. */
    li      t0, 0x80000000
    bne     a0, t0, riscv_do_div
    li      t0, -1
    beq     a1, t0, riscv_overflow

riscv_do_div:
    div     a0, a0, a1
    j       riscv_success

riscv_success:
    li      a1, 0
    ret

riscv_division_by_zero:
    li      a0, 0
    li      a1, 1
    ret

riscv_invalid_operation:
    li      a0, 0
    li      a1, 2
    ret

riscv_negative_factorial:
    li      a0, 0
    li      a1, 3
    ret

riscv_overflow:
    li      a0, 0
    li      a1, 4
    ret

    .section .rodata
    .align 2
riscv_test_cases:
    .word   5,  2, OP_ADD       /*  7; status 0 */
    .word   5,  7, OP_SUB       /* -2; status 0 */
    .word  -3,  2, OP_MUL       /* -6; status 0 */
    .word   5,  0, OP_FAT       /* 120; status 0 */
    .word   7,  2, OP_DIV       /*  3; status 0 */
    .word   7,  0, OP_DIV       /*  0; status 1 */
    .word  -1,  0, OP_FAT       /*  0; status 3 */
    .word  13,  0, OP_FAT       /*  0; status 4 */

    .section .bss
    .align 2
riscv_results:
    .space  RISCV_TEST_COUNT * 4
riscv_status:
    .space  RISCV_TEST_COUNT * 4
