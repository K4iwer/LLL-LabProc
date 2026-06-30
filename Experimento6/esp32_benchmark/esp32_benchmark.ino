#include <Arduino.h>
#include <esp_timer.h>
#include <math.h>
#include <stdint.h>

constexpr uint8_t NUM_AMOSTRAS = 30;
constexpr uint32_t REPETICOES_ARITMETICAS = 200000;
constexpr uint32_t REPETICOES_FATORIAL = 10000;

using Operacao = uint64_t (*)(uint64_t, uint64_t, uint64_t);

struct CasoBenchmark {
    const char *nome;
    Operacao funcao32;
    Operacao funcao64;
    uint32_t repeticoes;
    bool ehFatorial;
};

volatile uint64_t barreiraOtimizacao = 0;

uint64_t __attribute__((noinline))
soma32(uint64_t a, uint64_t b, uint64_t mascara) {
    return (static_cast<uint32_t>(a) + static_cast<uint32_t>(b)) &
           static_cast<uint32_t>(mascara);
}

uint64_t __attribute__((noinline))
soma64(uint64_t a, uint64_t b, uint64_t mascara) {
    return (a + b) & mascara;
}

uint64_t __attribute__((noinline))
subtracao32(uint64_t a, uint64_t b, uint64_t mascara) {
    return (static_cast<uint32_t>(a) - static_cast<uint32_t>(b)) &
           static_cast<uint32_t>(mascara);
}

uint64_t __attribute__((noinline))
subtracao64(uint64_t a, uint64_t b, uint64_t mascara) {
    return (a - b) & mascara;
}

uint64_t __attribute__((noinline))
multiplicacao32(uint64_t a, uint64_t b, uint64_t mascara) {
    return (static_cast<uint32_t>(a) * static_cast<uint32_t>(b)) &
           static_cast<uint32_t>(mascara);
}

uint64_t __attribute__((noinline))
multiplicacao64(uint64_t a, uint64_t b, uint64_t mascara) {
    return (a * b) & mascara;
}

uint64_t __attribute__((noinline))
divisao32(uint64_t a, uint64_t b, uint64_t mascara) {
    return (static_cast<uint32_t>(a) / static_cast<uint32_t>(b)) &
           static_cast<uint32_t>(mascara);
}

uint64_t __attribute__((noinline))
divisao64(uint64_t a, uint64_t b, uint64_t mascara) {
    return (a / b) & mascara;
}

uint64_t __attribute__((noinline))
fatorial32(uint64_t n, uint64_t, uint64_t mascara) {
    uint32_t resultado = 1;
    for (uint32_t fator = 2; fator <= static_cast<uint32_t>(n); ++fator) {
        resultado = (resultado * fator) & static_cast<uint32_t>(mascara);
    }
    return resultado;
}

uint64_t __attribute__((noinline))
fatorial64(uint64_t n, uint64_t, uint64_t mascara) {
    uint64_t resultado = 1;
    for (uint64_t fator = 2; fator <= n; ++fator) {
        resultado = (resultado * fator) & mascara;
    }
    return resultado;
}

void executarCaso(uint8_t bits, const CasoBenchmark &caso,
                  uint64_t a, uint64_t b, uint64_t mascara) {
    double media = 0.0;
    double somaQuadrados = 0.0;
    Operacao funcao = bits == 64 ? caso.funcao64 : caso.funcao32;

    for (uint8_t amostra = 1; amostra <= NUM_AMOSTRAS; ++amostra) {
        volatile uint64_t entradaA = a;
        volatile uint64_t entradaB = b;

        int64_t inicioUs = esp_timer_get_time();
        for (uint32_t i = 0; i < caso.repeticoes; ++i) {
            barreiraOtimizacao = funcao(entradaA, entradaB, mascara);
        }
        int64_t duracaoUs = esp_timer_get_time() - inicioUs;
        double nsPorOperacao =
            (static_cast<double>(duracaoUs) * 1000.0) / caso.repeticoes;

        double delta = nsPorOperacao - media;
        media += delta / amostra;
        somaQuadrados += delta * (nsPorOperacao - media);

        Serial.printf("AMOSTRA,ESP32-C3,%u,%s,%u,%lu,%.3f,\n",
                      static_cast<unsigned>(bits), caso.nome,
                      static_cast<unsigned>(amostra),
                      static_cast<unsigned long>(caso.repeticoes),
                      nsPorOperacao);
    }

    double desvioPadrao =
        sqrt(somaQuadrados / static_cast<double>(NUM_AMOSTRAS - 1));
    Serial.printf("RESUMO,ESP32-C3,%u,%s,0,%lu,%.3f,%.3f\n",
                  static_cast<unsigned>(bits), caso.nome,
                  static_cast<unsigned long>(caso.repeticoes),
                  media, desvioPadrao);
}

void setup() {
    Serial.begin(115200);
    delay(1500);

    static const uint8_t larguras[] = {4, 8, 16, 32, 64};
    static const uint64_t entradasFatorial[] = {4, 8, 12, 16, 20};
    static const CasoBenchmark casos[] = {
        {"soma", soma32, soma64, REPETICOES_ARITMETICAS, false},
        {"subtracao", subtracao32, subtracao64,
         REPETICOES_ARITMETICAS, false},
        {"multiplicacao", multiplicacao32, multiplicacao64,
         REPETICOES_ARITMETICAS, false},
        {"divisao", divisao32, divisao64,
         REPETICOES_ARITMETICAS, false},
        {"fatorial", fatorial32, fatorial64, REPETICOES_FATORIAL, true},
    };

    Serial.println(
        "registro,plataforma,bits,operacao,amostra,repeticoes,"
        "ns_por_operacao,desvio_padrao_ns");

    for (size_t largura = 0;
         largura < sizeof(larguras) / sizeof(larguras[0]); ++largura) {
        uint8_t bits = larguras[largura];
        uint64_t mascara =
            bits == 64 ? UINT64_MAX : (UINT64_C(1) << bits) - 1U;
        uint64_t a = (UINT64_C(1) << (bits - 2U)) - 1U;
        uint64_t b = (UINT64_C(1) << (bits - 3U)) + 1U;

        for (const CasoBenchmark &caso : casos) {
            if (caso.ehFatorial) {
                executarCaso(bits, caso, entradasFatorial[largura], 0,
                             mascara);
            } else {
                executarCaso(bits, caso, a, b, mascara);
            }
        }
    }

    Serial.println("# FIM");
}

void loop() {
    delay(1000);
}
