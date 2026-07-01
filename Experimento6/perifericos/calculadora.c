/**********************************************************************
 * Filename    : calculadora_lcd_keypad.cpp
 * Description : Calculadora binaria de 4 bits (complemento de dois)
 *               usando teclado matricial 4x4 e display LCD I2C 16x2
 *               no Raspberry Pi (baseado em MatrixKeypad.cpp e
 *               I2CLCD1602.c da Freenove).
 *
 * Compilar    : g++ -o calculadora calculadora_lcd_keypad.cpp \
 *                   -lwiringPi -lwiringPiDev -lpthread
 * Executar    : sudo ./calculadora
 **********************************************************************/
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <pcf8574.h>
#include <lcd.h>
#include "Keypad.hpp"

/* ---------------- Configuracao do teclado matricial 4x4 ---------------- */
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {16, 20, 21, 26};
byte colPins[COLS] = {19, 13, 6, 5};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

/* ---------------- Configuracao do LCD I2C 16x2 -------------------------- */
int pcf8574_address = 0x27; // PCF8574T:0x27, PCF8574AT:0x3F
#define BASE 64
#define RS  (BASE + 0)
#define RW  (BASE + 1)
#define EN  (BASE + 2)
#define LED (BASE + 3)
#define D4  (BASE + 4)
#define D5  (BASE + 5)
#define D6  (BASE + 6)
#define D7  (BASE + 7)
int lcdhd;

typedef enum {
    CALC_OK = 0,
    CALC_DIVISAO_POR_ZERO,
    CALC_FATORIAL_NEGATIVO,
    CALC_OPERACAO_INVALIDA
} CalcStatus;

/* ---------------- Funcoes auxiliares de LCD ------------------------------ */
static int detectI2C(int addr) {
    int fd = wiringPiI2CSetup(addr);
    if (fd < 0) {
        return 0;
    }
    if (wiringPiI2CWrite(fd, 0) < 0) {
        printf("Nao encontrado dispositivo no endereco 0x%x\n", addr);
        return 0;
    }
    printf("Dispositivo encontrado no endereco 0x%x\n", addr);
    return 1;
}

static bool lcdSetup(void) {
    if (detectI2C(0x27)) {
        pcf8574_address = 0x27;
    } else if (detectI2C(0x3F)) {
        pcf8574_address = 0x3F;
    } else {
        printf("Endereco I2C do LCD nao encontrado.\n"
               "Use 'i2cdetect -y 1' para verificar o endereco!\n");
        return false;
    }

    pcf8574Setup(BASE, pcf8574_address);
    for (int i = 0; i < 8; i++) {
        pinMode(BASE + i, OUTPUT);
    }
    digitalWrite(LED, HIGH); // liga o backlight
    digitalWrite(RW, LOW);   // habilita escrita

    lcdhd = lcdInit(2, 16, 4, RS, EN, D4, D5, D6, D7, 0, 0, 0, 0);
    if (lcdhd == -1) {
        printf("lcdInit falhou!\n");
        return false;
    }
    lcdClear(lcdhd);
    return true;
}

/* Escreve uma linha inteira do LCD, limpando o conteudo anterior */
static void lcdLine(int linha, const char *texto) {
    lcdPosition(lcdhd, 0, linha);
    lcdPuts(lcdhd, "                "); // 16 espacos
    lcdPosition(lcdhd, 0, linha);
    lcdPuts(lcdhd, texto);
}

/* ---------------- Logica da calculadora (igual ao original) ------------- */
static int32_t binario4_para_valor(const char bits[4]) {
    int32_t acumulado = 0;
    for (size_t i = 0; i < 4; ++i) {
        acumulado = (acumulado << 1) | (bits[i] - '0');
    }
    /* Interpreta os quatro bits em complemento de dois: faixa -8 a 7. */
    if (acumulado & 0x8) {
        acumulado -= 16;
    }
    return acumulado;
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

static void formatar_4bits(int64_t resultado, char saida[5]) {
    uint8_t quatro_bits = (uint8_t)resultado & 0x0FU;
    for (int bit = 3; bit >= 0; --bit) {
        saida[3 - bit] = (quatro_bits & (1U << bit)) ? '1' : '0';
    }
    saida[4] = '\0';
}

/* ---------------- Leitura de 4 bits pelo teclado matricial -------------- */
/* '#' cancela a leitura atual; '*' apaga o ultimo digito (backspace)      */
static bool lerBits4(const char *rotulo, char bits[4]) {
    int pos = 0;
    char cabecalho[17];
    snprintf(cabecalho, sizeof cabecalho, "%s:", rotulo);
    lcdLine(0, cabecalho);
    lcdLine(1, "");

    while (pos < 4) {
        char tecla = keypad.getKey();
        if (!tecla) {
            continue;
        }

        if (tecla == '#') {
            return false; // cancelar
        }
        if (tecla == '*') {
            if (pos > 0) {
                pos--;
                char mostrar[5] = {0};
                memcpy(mostrar, bits, pos);
                lcdLine(1, mostrar);
            }
            continue;
        }
        if (tecla != '0' && tecla != '1') {
            continue; // ignora teclas que nao sejam 0/1
        }

        bits[pos++] = tecla;
        printf("%c", tecla);
        fflush(stdout);

        char mostrar[5] = {0};
        memcpy(mostrar, bits, pos);
        lcdLine(1, mostrar);
    }
    putchar('\n');
    return true;
}

/* ---------------- Leitura da operacao pelo teclado ----------------------
 * A = +   B = -   C = *   D = /   * = !  (fatorial)   # = cancelar
 * ------------------------------------------------------------------------*/
static char lerOperacao(void) {
    lcdLine(0, "Operacao:");
    lcdLine(1, "A+ B- C* D/ *!");

    while (true) {
        char tecla = keypad.getKey();
        if (!tecla) {
            continue;
        }
        switch (tecla) {
            case 'A': return '+';
            case 'B': return '-';
            case 'C': return '*';
            case 'D': return '/';
            case '*': return '!';
            case '#': return 0; // cancelar
            default: break;
        }
    }
}

static void mostrarResultado(int64_t resultado) {
    char bits[5];
    formatar_4bits(resultado, bits);
    bool overflow = (resultado < -8 || resultado > 7);

    char linha0[17];
    snprintf(linha0, sizeof linha0, "Dec:%" PRId64, resultado);
    lcdLine(0, linha0);

    char linha1[17];
    if (overflow) {
        snprintf(linha1, sizeof linha1, "%s OVERFLOW", bits);
    } else {
        snprintf(linha1, sizeof linha1, "Bin4:%s", bits);
    }
    lcdLine(1, linha1);

    printf("\nResultado decimal: %" PRId64 "\n", resultado);
    printf("Resultado nos 4 bits de saida: %s\n", bits);
    if (overflow) {
        puts("Aviso: overflow na saida assinada de 4 bits.");
    }
}

static void mostrarErro(const char *msgLcd, const char *msgTerminal) {
    lcdLine(0, "Erro:");
    lcdLine(1, msgLcd);
    printf("\nErro: %s\n", msgTerminal);
}

/* ---------------------------------- main --------------------------------- */
int main(void) {
    printf("Calculadora binaria de 4 bits - LCD + Keypad (Raspberry Pi)\n");

    /* wiringPiSetupGpio usa numeracao BCM, necessaria para os pinos do
       keypad. O LCD por I2C nao e afetado por essa escolha, pois usa
       /dev/i2c-1 diretamente e pinos virtuais (BASE+0..7) fora da faixa
       de GPIOs fisicos. */
    wiringPiSetupGpio();
    keypad.setDebounceTime(50);

    if (!lcdSetup()) {
        return 1;
    }

    lcdLine(0, "Calculadora 4b");
    lcdLine(1, "Pressione tecla");
    delay(1500);

    while (true) {
        char bitsA[4];
        char bitsB[4];

        if (!lerBits4("A (4 bits)", bitsA)) {
            lcdLine(0, "Cancelado");
            lcdLine(1, "");
            delay(800);
            continue;
        }
        int32_t a = binario4_para_valor(bitsA);

        char operacao = lerOperacao();
        if (operacao == 0) {
            lcdLine(0, "Cancelado");
            lcdLine(1, "");
            delay(800);
            continue;
        }

        int32_t b = 0;
        if (operacao != '!') {
            if (!lerBits4("B (4 bits)", bitsB)) {
                lcdLine(0, "Cancelado");
                lcdLine(1, "");
                delay(800);
                continue;
            }
            b = binario4_para_valor(bitsB);
        }

        int64_t resultado = 0;
        CalcStatus status = calcular(a, b, operacao, &resultado);
        switch (status) {
            case CALC_OK:
                mostrarResultado(resultado);
                break;
            case CALC_DIVISAO_POR_ZERO:
                mostrarErro("Divisao por 0", "divisao por zero. Nenhuma divisao foi executada.");
                break;
            case CALC_FATORIAL_NEGATIVO:
                mostrarErro("Fatorial neg.", "o fatorial nao esta definido para inteiros negativos.");
                break;
            case CALC_OPERACAO_INVALIDA:
                mostrarErro("Op. invalida", "operacao invalida.");
                break;
        }
        delay(3000); // tempo para ler o resultado no LCD antes da proxima rodada
    }

    return 0;
}