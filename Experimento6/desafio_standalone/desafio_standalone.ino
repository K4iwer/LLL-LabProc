#include <iostream>
#include <string>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <wiringPi.h>
#include <pcf8574.h>
#include <lcd.h>

// Configurações do Display LCD I2C
const int PCF_BASE = 64; // Base arbitrária para os pinos do expansor I2C
const int I2C_ADDR = 0x27; // Endereço padrão do display, pode ser 0x3F
int lcdFd; // File descriptor do LCD

// Configurações do Teclado Matricial
constexpr byte NUM_LINHAS = 4;
constexpr byte NUM_COLUNAS = 4;

// Pinos BCM da Raspberry Pi (Altere de acordo com a sua montagem)
const int pinosLinhas[NUM_LINHAS] = {5, 6, 13, 19}; 
const int pinosColunas[NUM_COLUNAS] = {12, 16, 20, 21};

char teclas[NUM_LINHAS][NUM_COLUNAS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'},
};

std::string entradaA;
std::string entradaB;
char operacao = '\0';
bool exibindoResultado = false;

// Função para ler o teclado matricial "na mão"
char getKey() {
    for(int c = 0; c < NUM_COLUNAS; c++) pinMode(pinosColunas[c], OUTPUT);
    for(int r = 0; r < NUM_LINHAS; r++) { 
        pinMode(pinosLinhas[r], INPUT); 
        pullUpDnControl(pinosLinhas[r], PUD_UP); 
    }

    for(int c = 0; c < NUM_COLUNAS; c++) {
        digitalWrite(pinosColunas[c], LOW);
        for(int r = 0; r < NUM_LINHAS; r++) {
            if(digitalRead(pinosLinhas[r]) == LOW) {
                delay(50); // debounce
                while(digitalRead(pinosLinhas[r]) == LOW); // espera soltar a tecla
                digitalWrite(pinosColunas[c], HIGH);
                return teclas[r][c];
            }
        }
        digitalWrite(pinosColunas[c], HIGH);
    }
    return '\0';
}

void imprimirLinha(int linha, const std::string &texto) {
    lcdPosition(lcdFd, 0, linha);
    lcdPuts(lcdFd, "                ");
    lcdPosition(lcdFd, 0, linha);
    lcdPuts(lcdFd, texto.substr(0, 16).c_str());
}

int binario4Assinado(const std::string &binario) {
    int valor = static_cast<int>(strtol(binario.c_str(), nullptr, 2));
    return valor >= 8 ? valor - 16 : valor;
}

std::string binario4(int64_t valor) {
    uint8_t quatroBits = static_cast<uint8_t>(valor) & 0x0F;
    std::string texto;
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
    std::string linhaA = "A:" + entradaA;
    while (linhaA.length() < 6) linhaA += '_';

    std::string linhaB;
    if (operacao == '\0') {
        linhaB = "Escolha operacao";
    } else if (operacao == '!') {
        linhaB = "Fatorial  #=OK";
    } else {
        linhaB = std::string(1, operacao) + " B:" + entradaB;
        while (linhaB.length() < 8) linhaB += '_';
        linhaB += " #=OK";
    }

    imprimirLinha(0, linhaA);
    imprimirLinha(1, linhaB);
}

void mostrarErro(const std::string &linha1, const std::string &linha2) {
    imprimirLinha(0, linha1);
    imprimirLinha(1, linha2);
    exibindoResultado = true;
}

int64_t calcularFatorial(int valor) {
    int64_t resultado = 1;
    for (int fator = 2; fator <= valor; ++fator) resultado *= fator;
    return resultado;
}

void executarCalculo() {
    if (entradaA.length() != 4 || (operacao != '!' && entradaB.length() != 4) || operacao == '\0') {
        mostrarErro("Entrada incompleta", "0/1 reinicia");
        return;
    }

    int a = binario4Assinado(entradaA);
    int b = operacao == '!' ? 0 : binario4Assinado(entradaB);
    int64_t resultado = 0;

    switch (operacao) {
        case '+': resultado = static_cast<int64_t>(a) + b; break;
        case '-': resultado = static_cast<int64_t>(a) - b; break;
        case 'x': resultado = static_cast<int64_t>(a) * b; break;
        case '/':
            if (b == 0) { mostrarErro("Erro: divisao", "por zero"); return; }
            resultado = a / b;
            break;
        case '!':
            if (a < 0) { mostrarErro("Erro: fatorial", "valor negativo"); return; }
            resultado = calcularFatorial(a);
            break;
    }

    std::string linhaResultado = "R:" + binario4(resultado) + " D:" + std::to_string((long)resultado);
    imprimirLinha(0, linhaResultado);
    imprimirLinha(1, resultado < -8 || resultado > 7 ? "OVERFLOW 4 bits" : "Resultado valido");
    exibindoResultado = true;

    printf("A=%s op=%c B=%s resultado=%lld bits=%s%s\n",
           entradaA.c_str(), operacao, entradaB.c_str(),
           static_cast<long long>(resultado), binario4(resultado).c_str(),
           resultado < -8 || resultado > 7 ? " OVERFLOW" : "");
}

void selecionarOperacao(char tecla) {
    if (entradaA.length() != 4) { mostrarErro("Complete A: 4bit", "0/1 reinicia"); return; }

    switch (tecla) {
        case 'A': operacao = '+'; break;
        case 'B': operacao = '-'; break;
        case 'C': operacao = 'x'; break;
        case 'D': operacao = '/'; break;
        case '*': operacao = '!'; break;
    }
    entradaB = "";
    mostrarEntrada();
}

void setup() {
    // Inicializa wiringPi usando a numeração padrão do BCM GPIO
    wiringPiSetupGpio(); 

    // Inicializa o I2C com o LCD via PCF8574
    pcf8574Setup(PCF_BASE, I2C_ADDR);
    lcdFd = lcdInit(2, 16, 4, 
                    PCF_BASE + 0, PCF_BASE + 2, PCF_BASE + 4, PCF_BASE + 5, 
                    PCF_BASE + 6, PCF_BASE + 7, 0, 0, 0, 0);
                    
    // Liga a luz de fundo do LCD (Pino 3 do PCF8574)
    pinMode(PCF_BASE + 3, OUTPUT);
    digitalWrite(PCF_BASE + 3, HIGH);

    limparCalculo();
    mostrarEntrada();
}

int main() {
    setup();

    while (true) {
        char tecla = getKey();
        if (tecla == '\0') {
            delay(50);
            continue;
        }

        if (tecla == '0' || tecla == '1') {
            if (exibindoResultado) limparCalculo();

            std::string &entradaAtual = operacao == '\0' ? entradaA : entradaB;
            if (operacao == '!') {
                mostrarErro("Fatorial sem B", "# para executar");
                continue;
            }
            if (entradaAtual.length() < 4) entradaAtual += tecla;
            mostrarEntrada();
            continue;
        }

        if (tecla == '#') {
            if (exibindoResultado) {
                limparCalculo();
                mostrarEntrada();
            } else {
                executarCalculo();
            }
            continue;
        }

        if (tecla == 'A' || tecla == 'B' || tecla == 'C' || tecla == 'D' || tecla == '*') {
            if (exibindoResultado) {
                mostrarErro("Digite novo A", "usando 0 e 1");
                continue;
            }
            selecionarOperacao(tecla);
        }
    }
    return 0;
}