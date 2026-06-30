#define _POSIX_C_SOURCE 200809L

#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#if defined(__GNUC__)
#define NOINLINE __attribute__((noinline))
#else
#define NOINLINE
#endif

enum {
    NUM_AMOSTRAS = 30,
    REPETICOES_ARITMETICAS = 200000,
    REPETICOES_FATORIAL = 10000
};

typedef uint64_t (*Operacao)(uint64_t, uint64_t, uint64_t);

typedef struct {
    const char *nome;
    Operacao funcao32;
    Operacao funcao64;
    uint32_t repeticoes;
    int eh_fatorial;
} CasoBenchmark;

static volatile uint64_t barreira_otimizacao;

static NOINLINE uint64_t soma32(uint64_t a, uint64_t b, uint64_t mascara) {
    return ((uint32_t)a + (uint32_t)b) & (uint32_t)mascara;
}

static NOINLINE uint64_t soma64(uint64_t a, uint64_t b, uint64_t mascara) {
    return (a + b) & mascara;
}

static NOINLINE uint64_t subtracao32(uint64_t a, uint64_t b,
                                    uint64_t mascara) {
    return ((uint32_t)a - (uint32_t)b) & (uint32_t)mascara;
}

static NOINLINE uint64_t subtracao64(uint64_t a, uint64_t b,
                                    uint64_t mascara) {
    return (a - b) & mascara;
}

static NOINLINE uint64_t multiplicacao32(uint64_t a, uint64_t b,
                                        uint64_t mascara) {
    return ((uint32_t)a * (uint32_t)b) & (uint32_t)mascara;
}

static NOINLINE uint64_t multiplicacao64(uint64_t a, uint64_t b,
                                        uint64_t mascara) {
    return (a * b) & mascara;
}

static NOINLINE uint64_t divisao32(uint64_t a, uint64_t b,
                                  uint64_t mascara) {
    return ((uint32_t)a / (uint32_t)b) & (uint32_t)mascara;
}

static NOINLINE uint64_t divisao64(uint64_t a, uint64_t b,
                                  uint64_t mascara) {
    return (a / b) & mascara;
}

static NOINLINE uint64_t fatorial32(uint64_t n, uint64_t ignorado,
                                   uint64_t mascara) {
    (void)ignorado;
    uint32_t resultado = 1;
    for (uint32_t fator = 2; fator <= (uint32_t)n; ++fator) {
        resultado = (resultado * fator) & (uint32_t)mascara;
    }
    return resultado;
}

static NOINLINE uint64_t fatorial64(uint64_t n, uint64_t ignorado,
                                   uint64_t mascara) {
    (void)ignorado;
    uint64_t resultado = 1;
    for (uint64_t fator = 2; fator <= n; ++fator) {
        resultado = (resultado * fator) & mascara;
    }
    return resultado;
}

static uint64_t nanos_agora(void) {
    struct timespec instante;
#ifdef CLOCK_MONOTONIC_RAW
    clock_gettime(CLOCK_MONOTONIC_RAW, &instante);
#else
    clock_gettime(CLOCK_MONOTONIC, &instante);
#endif
    return (uint64_t)instante.tv_sec * UINT64_C(1000000000) +
           (uint64_t)instante.tv_nsec;
}

static void executar_caso(unsigned bits, const CasoBenchmark *caso,
                         uint64_t a, uint64_t b, uint64_t mascara) {
    double media = 0.0;
    double soma_quadrados = 0.0;
    Operacao funcao = bits == 64 ? caso->funcao64 : caso->funcao32;

    for (unsigned amostra = 1; amostra <= NUM_AMOSTRAS; ++amostra) {
        volatile uint64_t entrada_a = a;
        volatile uint64_t entrada_b = b;

        uint64_t inicio = nanos_agora();
        for (uint32_t i = 0; i < caso->repeticoes; ++i) {
            barreira_otimizacao =
                funcao(entrada_a, entrada_b, mascara);
        }
        uint64_t duracao = nanos_agora() - inicio;
        double ns_por_operacao = (double)duracao / caso->repeticoes;

        double delta = ns_por_operacao - media;
        media += delta / amostra;
        soma_quadrados += delta * (ns_por_operacao - media);

        printf("AMOSTRA,RaspberryPi3,%u,%s,%u,%" PRIu32 ",%.3f,\n",
               bits, caso->nome, amostra, caso->repeticoes,
               ns_por_operacao);
    }

    double desvio_padrao =
        sqrt(soma_quadrados / (double)(NUM_AMOSTRAS - 1));
    printf("RESUMO,RaspberryPi3,%u,%s,0,%" PRIu32 ",%.3f,%.3f\n",
           bits, caso->nome, caso->repeticoes, media, desvio_padrao);
}

int main(void) {
    static const unsigned larguras[] = {4, 8, 16, 32, 64};
    static const uint64_t entradas_fatorial[] = {4, 8, 12, 16, 20};
    static const CasoBenchmark casos[] = {
        {"soma", soma32, soma64, REPETICOES_ARITMETICAS, 0},
        {"subtracao", subtracao32, subtracao64,
         REPETICOES_ARITMETICAS, 0},
        {"multiplicacao", multiplicacao32, multiplicacao64,
         REPETICOES_ARITMETICAS, 0},
        {"divisao", divisao32, divisao64, REPETICOES_ARITMETICAS, 0},
        {"fatorial", fatorial32, fatorial64, REPETICOES_FATORIAL, 1},
    };

    puts("registro,plataforma,bits,operacao,amostra,repeticoes,"
         "ns_por_operacao,desvio_padrao_ns");

    for (size_t largura = 0;
         largura < sizeof larguras / sizeof larguras[0]; ++largura) {
        unsigned bits = larguras[largura];
        uint64_t mascara =
            bits == 64 ? UINT64_MAX : (UINT64_C(1) << bits) - 1U;
        uint64_t a = (UINT64_C(1) << (bits - 2U)) - 1U;
        uint64_t b = (UINT64_C(1) << (bits - 3U)) + 1U;

        for (size_t caso = 0; caso < sizeof casos / sizeof casos[0]; ++caso) {
            if (casos[caso].eh_fatorial) {
                executar_caso(bits, &casos[caso],
                              entradas_fatorial[largura], 0, mascara);
            } else {
                executar_caso(bits, &casos[caso], a, b, mascara);
            }
        }
    }

    return barreira_otimizacao == UINT64_MAX;
}
