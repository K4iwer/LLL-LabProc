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
int lcdhd;// used to handle LCD
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
    lcdPosition(lcdhd,0,1);// set the LCD cursor position to (0,1)
    lcdPrintf("\nResultado decimal: %" PRId64 "\n", resultado);
    
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
int main(void)
{
    char entrada[32];

    printf("Calculadora binaria iniciando...\n");

    if (!inicializarHardware()) {
        printf("Falha na inicializacao do hardware\n");
        return -1;
    }

    lcdClear(lcdhd);

    while (true)
    {
        int32_t a;
        int32_t b = 0;
        int64_t resultado = 0;

        char bitsA[5];
        char bitsB[5];
        char operacao = 0;

        /* =========================
         * LEITURA DE A (4 bits)
         * ========================= */
        lcdClear(lcdhd);
        lcdPosition(lcdhd, 0, 0);
        lcdPrintf(lcdhd, "A (4 bits):");

        ler4Bits(bitsA);  // usa keypad

        if (strcmp(bitsA, "q") == 0 || strcmp(bitsA, "Q") == 0)
            break;

        if (!binario4_para_decimal(bitsA, &a)) {
            lcdClear(lcdhd);
            lcdPrintf(lcdhd, "Entrada invalida");
            delay(1500);
            continue;
        }

        /* =========================
         * LEITURA OPERACAO
         * ========================= */
        lcdClear(lcdhd);
        lcdPrintf(lcdhd, "+ - * / !");

        operacao = lerOperacao();  // keypad

        /* =========================
         * LEITURA DE B (se precisar)
         * ========================= */
        if (operacao != '!')
        {
            lcdClear(lcdhd);
            lcdPrintf(lcdhd, "B (4 bits):");

            ler4Bits(bitsB);

            if (!binario4_para_decimal(bitsB, &b)) {
                lcdClear(lcdhd);
                lcdPrintf(lcdhd, "Entrada invalida");
                delay(1500);
                continue;
            }
        }

        /* =========================
         * CALCULO
         * ========================= */
        CalcStatus status = calcular(a, b, operacao, &resultado);

        lcdClear(lcdhd);

        switch (status)
        {
            case CALC_OK:
            {
                char linha1[17];
                char linha2[17];

                sprintf(linha1, "Res:%ld", (long)resultado);

                uint8_t q = resultado & 0x0F;

                sprintf(linha2, "%d%d%d%d",
                        (q >> 3) & 1,
                        (q >> 2) & 1,
                        (q >> 1) & 1,
                        (q >> 0) & 1);

                lcdPosition(lcdhd, 0, 0);
                lcdPrintf(lcdhd, "%s", linha1);

                lcdPosition(lcdhd, 0, 1);
                lcdPrintf(lcdhd, "%s", linha2);

                break;
            }

            case CALC_DIVISAO_POR_ZERO:
                lcdPrintf(lcdhd, "Erro: /0");
                break;

            case CALC_FATORIAL_NEGATIVO:
                lcdPrintf(lcdhd, "Erro: fat neg");
                break;

            case CALC_OPERACAO_INVALIDA:
                lcdPrintf(lcdhd, "Operacao invalida");
                break;
        }

        delay(2500);
    }

    lcdClear(lcdhd);
    lcdPrintf(lcdhd, "Fim");

    printf("Calculadora encerrada.\n");
    return 0;
}