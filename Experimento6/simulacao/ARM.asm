/*
 * Calculadora equivalente em ARM A32 (Cortex-A53 em modo AArch32).
 *
 * Entradas de calc_arm:
 *   r0 = operando A
 *   r1 = operando B
 *   r2 = operacao: 0 soma, 1 subtracao, 2 multiplicacao,
 *                  3 fatorial, 4 divisao
 *
 * Saidas:
 *   r0 = resultado
 *   r1 = status: 0 sucesso, 1 divisao por zero, 2 operacao invalida,
 *                3 fatorial negativo, 4 overflow
 *
 * O bloco _start executa casos de teste automaticamente. No simulador,
 * inspecione os vetores arm_results e arm_status depois que o programa
 * chegar a arm_stop.
 */

    .syntax unified
    .cpu cortex-a53
    .arm

    .equ OP_ADD, 0
    .equ OP_SUB, 1
    .equ OP_MUL, 2
    .equ OP_FAT, 3
    .equ OP_DIV, 4
    .equ ARM_TEST_COUNT, 8

    .section .text
    .global _start
    .global calc_arm

_start:
    ldr     r4, =arm_test_cases
    ldr     r5, =arm_results
    ldr     r6, =arm_status
    mov     r7, #ARM_TEST_COUNT

arm_test_loop:
    ldr     r0, [r4], #4
    ldr     r1, [r4], #4
    ldr     r2, [r4], #4
    bl      calc_arm
    str     r0, [r5], #4
    str     r1, [r6], #4
    subs    r7, r7, #1
    bne     arm_test_loop

arm_stop:
    b       arm_stop

calc_arm:
    push    {r4-r6, lr}

    cmp     r2, #OP_ADD
    beq     arm_add
    cmp     r2, #OP_SUB
    beq     arm_sub
    cmp     r2, #OP_MUL
    beq     arm_mul
    cmp     r2, #OP_FAT
    beq     arm_factorial
    cmp     r2, #OP_DIV
    beq     arm_div
    b       arm_invalid_operation

arm_add:
    adds    r0, r0, r1
    bvs     arm_overflow
    b       arm_success

arm_sub:
    subs    r0, r0, r1
    bvs     arm_overflow
    b       arm_success

arm_mul:
    smull   r4, r5, r0, r1
    mov     r6, r4, asr #31
    cmp     r5, r6
    bne     arm_overflow
    mov     r0, r4
    b       arm_success

arm_factorial:
    cmp     r0, #0
    blt     arm_negative_factorial
    cmp     r0, #12
    bgt     arm_overflow

    mov     r4, r0
    mov     r0, #1
    cmp     r4, #1
    ble     arm_success

arm_factorial_loop:
    mul     r0, r0, r4
    subs    r4, r4, #1
    cmp     r4, #1
    bgt     arm_factorial_loop
    b       arm_success

arm_div:
    cmp     r1, #0
    beq     arm_division_by_zero

    /* INT32_MIN / -1 nao cabe em um inteiro assinado de 32 bits. */
    ldr     r4, =0x80000000
    cmp     r0, r4
    bne     arm_do_div
    cmn     r1, #1
    beq     arm_overflow

arm_do_div:
    sdiv    r0, r0, r1
    b       arm_success

arm_success:
    mov     r1, #0
    pop     {r4-r6, pc}

arm_division_by_zero:
    mov     r0, #0
    mov     r1, #1
    pop     {r4-r6, pc}

arm_invalid_operation:
    mov     r0, #0
    mov     r1, #2
    pop     {r4-r6, pc}

arm_negative_factorial:
    mov     r0, #0
    mov     r1, #3
    pop     {r4-r6, pc}

arm_overflow:
    mov     r0, #0
    mov     r1, #4
    pop     {r4-r6, pc}

    .section .rodata
    .align 2
arm_test_cases:
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
arm_results:
    .space  ARM_TEST_COUNT * 4
arm_status:
    .space  ARM_TEST_COUNT * 4
