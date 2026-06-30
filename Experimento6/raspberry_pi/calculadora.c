#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef enum {
    CALC_OK = 0,
    CALC_DIVISAO_POR_ZERO,
    CALC_FATORIAL_NEGATIVO,
    CALC_OPERACAO_INVALIDA
} CalcStatus;

static bool ler_linha(char *destino, size_t tamanho) {
    if (fgets(destino, (int)tamanho, stdin) == NULL) {
        return false;
    }

    destino[strcspn(destino, "\r\n")] = '\0';
    return true;
}

static bool binario4_para_decimal(const char *texto, int32_t *valor) {
    if (strlen(texto) != 4U) {
        return false;
    }

    int32_t acumulado = 0;
    for (size_t i = 0; i < 4U; ++i) {
        if (texto[i] != '0' && texto[i] != '1') {
            return false;
        }
        acumulado = (acumulado << 1) | (texto[i] - '0');
    }

    /* Interpreta os quatro bits em complemento de dois: faixa -8 a 7. */
    if ((acumulado & 0x8) != 0) {
        acumulado -= 16;
    }

    *valor = acumulado;
    return true;
}

static void imprimir_binario_sem_sinal(uint64_t valor) {
    bool iniciou = false;

    for (int bit = 63; bit >= 0; --bit) {
        bool ligado = (valor & (UINT64_C(1) << bit)) != 0U;
        if (ligado || iniciou || bit == 0) {
            putchar(ligado ? '1' : '0');
            iniciou = true;
        }
    }
}

static void imprimir_resultado(int64_t resultado) {
    uint8_t quatro_bits = (uint8_t)resultado & 0x0FU;
    uint64_t magnitude;

    if (resultado < 0) {
        putchar('-');
        magnitude = (uint64_t)(-(resultado + 1)) + 1U;
    } else {
        magnitude = (uint64_t)resultado;
    }
    imprimir_binario_sem_sinal(magnitude);

    printf("\nResultado decimal: %" PRId64 "\n", resultado);
    printf("Resultado nos 4 bits de saida: ");
    for (int bit = 3; bit >= 0; --bit) {
        putchar((quatro_bits & (1U << bit)) != 0U ? '1' : '0');
    }
    putchar('\n');

    if (resultado < -8 || resultado > 7) {
        puts("Aviso: overflow na saida assinada de 4 bits.");
    }
}

static int64_t fatorial(int32_t valor) {
    int64_t resultado = 1;
    for (int32_t fator = 2; fator <= valor; ++fator) {
        resultado *= fator;
    }
    return resultado;
}

static CalcStatus calcular(int32_t a, int32_t b, char operacao,
                           int64_t *resultado) {
    switch (operacao) {
        case '+':
            *resultado = (int64_t)a + b;
            return CALC_OK;
        case '-':
            *resultado = (int64_t)a - b;
            return CALC_OK;
        case '*':
            *resultado = (int64_t)a * b;
            return CALC_OK;
        case '/':
            if (b == 0) {
                return CALC_DIVISAO_POR_ZERO;
            }
            *resultado = a / b;
            return CALC_OK;
        case '!':
            if (a < 0) {
                return CALC_FATORIAL_NEGATIVO;
            }
            *resultado = fatorial(a);
            return CALC_OK;
        default:
            return CALC_OPERACAO_INVALIDA;
    }
}

int main(void) {
    char entrada[32];

    puts("Calculadora binaria de 4 bits - Raspberry Pi 3");
    puts("Entradas em complemento de dois (-8 a 7). Digite q para sair.\n");

    while (true) {
        int32_t a;
        int32_t b = 0;
        int64_t resultado = 0;

        printf("A (4 bits): ");
        if (!ler_linha(entrada, sizeof entrada)) {
            break;
        }
        if (strcmp(entrada, "q") == 0 || strcmp(entrada, "Q") == 0) {
            break;
        }
        if (!binario4_para_decimal(entrada, &a)) {
            puts("Entrada invalida. Use exatamente quatro caracteres 0 ou 1.\n");
            continue;
        }

        printf("Operacao (+, -, *, /, !): ");
        if (!ler_linha(entrada, sizeof entrada)) {
            break;
        }
        char operacao = entrada[0];

        if (operacao != '!') {
            printf("B (4 bits): ");
            if (!ler_linha(entrada, sizeof entrada)) {
                break;
            }
            if (!binario4_para_decimal(entrada, &b)) {
                puts("Entrada invalida. Use exatamente quatro caracteres 0 ou 1.\n");
                continue;
            }
        }

        CalcStatus status = calcular(a, b, operacao, &resultado);
        switch (status) {
            case CALC_OK:
                printf("\nResultado binario exato: ");
                imprimir_resultado(resultado);
                break;
            case CALC_DIVISAO_POR_ZERO:
                puts("\nErro: divisao por zero. Nenhuma divisao foi executada.");
                break;
            case CALC_FATORIAL_NEGATIVO:
                puts("\nErro: o fatorial nao esta definido para inteiros negativos.");
                break;
            case CALC_OPERACAO_INVALIDA:
                puts("\nErro: operacao invalida.");
                break;
        }
        putchar('\n');
    }

    puts("Calculadora encerrada.");
    return 0;
}
