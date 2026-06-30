#include <Arduino.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

/*
 * Ligacoes sugeridas para ESP32-C3:
 *   LCD I2C: SDA = GPIO0, SCL = GPIO1, endereco 0x27
 *   Teclado:  linhas = GPIO2..5, colunas = GPIO6..9
 *
 * Teclas: A soma, B subtracao, C multiplicacao, D divisao,
 *         * fatorial e # executar.
 */

constexpr byte NUM_LINHAS = 4;
constexpr byte NUM_COLUNAS = 4;
constexpr int PINO_SDA = 0;
constexpr int PINO_SCL = 1;

char teclas[NUM_LINHAS][NUM_COLUNAS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'},
};

byte pinosLinhas[NUM_LINHAS] = {2, 3, 4, 5};
byte pinosColunas[NUM_COLUNAS] = {6, 7, 8, 9};

Keypad teclado =
    Keypad(makeKeymap(teclas), pinosLinhas, pinosColunas,
           NUM_LINHAS, NUM_COLUNAS);
LiquidCrystal_I2C lcd(0x27, 16, 2);

String entradaA;
String entradaB;
char operacao = '\0';
bool exibindoResultado = false;

void imprimirLinha(byte linha, const String &texto) {
    lcd.setCursor(0, linha);
    lcd.print("                ");
    lcd.setCursor(0, linha);
    lcd.print(texto.substring(0, 16));
}

int binario4Assinado(const String &binario) {
    int valor = static_cast<int>(strtol(binario.c_str(), nullptr, 2));
    return valor >= 8 ? valor - 16 : valor;
}

String binario4(int64_t valor) {
    uint8_t quatroBits = static_cast<uint8_t>(valor) & 0x0F;
    String texto;
    for (int bit = 3; bit >= 0; --bit) {
        texto += (quatroBits & (1U << bit)) != 0U ? '1' : '0';
    }
    return texto;
}

void limparCalculo() {
    entradaA = "";
    entradaB = "";
    operacao = '\0';
    exibindoResultado = false;
}

void mostrarEntrada() {
    String linhaA = "A:" + entradaA;
    while (linhaA.length() < 6) {
        linhaA += '_';
    }

    String linhaB;
    if (operacao == '\0') {
        linhaB = "Escolha operacao";
    } else if (operacao == '!') {
        linhaB = "Fatorial  #=OK";
    } else {
        linhaB = String(operacao) + " B:" + entradaB;
        while (linhaB.length() < 8) {
            linhaB += '_';
        }
        linhaB += " #=OK";
    }

    imprimirLinha(0, linhaA);
    imprimirLinha(1, linhaB);
}

void mostrarErro(const String &linha1, const String &linha2) {
    imprimirLinha(0, linha1);
    imprimirLinha(1, linha2);
    exibindoResultado = true;
}

int64_t calcularFatorial(int valor) {
    int64_t resultado = 1;
    for (int fator = 2; fator <= valor; ++fator) {
        resultado *= fator;
    }
    return resultado;
}

void executarCalculo() {
    if (entradaA.length() != 4 ||
        (operacao != '!' && entradaB.length() != 4) ||
        operacao == '\0') {
        mostrarErro("Entrada incompleta", "0/1 reinicia");
        return;
    }

    int a = binario4Assinado(entradaA);
    int b = operacao == '!' ? 0 : binario4Assinado(entradaB);
    int64_t resultado = 0;

    switch (operacao) {
        case '+':
            resultado = static_cast<int64_t>(a) + b;
            break;
        case '-':
            resultado = static_cast<int64_t>(a) - b;
            break;
        case 'x':
            resultado = static_cast<int64_t>(a) * b;
            break;
        case '/':
            if (b == 0) {
                mostrarErro("Erro: divisao", "por zero");
                return;
            }
            resultado = a / b;
            break;
        case '!':
            if (a < 0) {
                mostrarErro("Erro: fatorial", "valor negativo");
                return;
            }
            resultado = calcularFatorial(a);
            break;
    }

    String linhaResultado =
        "R:" + binario4(resultado) + " D:" + String((long)resultado);
    imprimirLinha(0, linhaResultado);
    imprimirLinha(1, resultado < -8 || resultado > 7
                         ? "OVERFLOW 4 bits"
                         : "Resultado valido");
    exibindoResultado = true;

    Serial.printf("A=%s op=%c B=%s resultado=%lld bits=%s%s\n",
                  entradaA.c_str(), operacao, entradaB.c_str(),
                  static_cast<long long>(resultado),
                  binario4(resultado).c_str(),
                  resultado < -8 || resultado > 7 ? " OVERFLOW" : "");
}

void selecionarOperacao(char tecla) {
    if (entradaA.length() != 4) {
        mostrarErro("Complete A: 4bit", "0/1 reinicia");
        return;
    }

    switch (tecla) {
        case 'A':
            operacao = '+';
            break;
        case 'B':
            operacao = '-';
            break;
        case 'C':
            operacao = 'x';
            break;
        case 'D':
            operacao = '/';
            break;
        case '*':
            operacao = '!';
            break;
    }
    entradaB = "";
    mostrarEntrada();
}

void setup() {
    Serial.begin(115200);
    Wire.begin(PINO_SDA, PINO_SCL);
    lcd.init();
    lcd.backlight();

    limparCalculo();
    mostrarEntrada();
}

void loop() {
    char tecla = teclado.getKey();
    if (tecla == NO_KEY) {
        return;
    }

    if (tecla == '0' || tecla == '1') {
        if (exibindoResultado) {
            limparCalculo();
        }

        String &entradaAtual = operacao == '\0' ? entradaA : entradaB;
        if (operacao == '!') {
            mostrarErro("Fatorial sem B", "# para executar");
            return;
        }
        if (entradaAtual.length() < 4) {
            entradaAtual += tecla;
        }
        mostrarEntrada();
        return;
    }

    if (tecla == '#') {
        if (exibindoResultado) {
            limparCalculo();
            mostrarEntrada();
        } else {
            executarCalculo();
        }
        return;
    }

    if (tecla == 'A' || tecla == 'B' || tecla == 'C' ||
        tecla == 'D' || tecla == '*') {
        if (exibindoResultado) {
            mostrarErro("Digite novo A", "usando 0 e 1");
            return;
        }
        selecionarOperacao(tecla);
    }
}
