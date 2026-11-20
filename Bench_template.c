/*
 * Template do programa de benchmark.
 *
 * Cada esquema possui uma copia deste arquivo, alterada apenas nos
 * pontos marcados com [ADAPTAR]. As funcoes medidas sao as mesmas do
 * benchmark original de cada submissao.
 */

#include <stdio.h>
#include <stdlib.h>

/* [ADAPTAR] cabecalhos do esquema avaliado */
#include "ALGORITMO.h"

#include "defines.h"
#include "clocks.h"

/* defines.h deve ativar exatamente um de GEN, SIGN ou VER */
#if (defined(GEN) + defined(SIGN) + defined(VER)) != 1
#error "defines.h: ative EXATAMENTE UM de GEN, SIGN ou VER"
#endif

/* [ADAPTAR] BENCH calibrado por operacao na categoria 5 */
#if defined(GEN)
#define BENCH 50
#elif defined(SIGN)
#define BENCH 50
#elif defined(VER)
#define BENCH 50
#endif

#define MSG_LEN 8

/* [ADAPTAR] tipos da API do esquema */
static pubkey_t pk;
static prikey_t sk;

#if defined(SIGN) || defined(VER)
static signature_t signature;
static unsigned char message[MSG_LEN] = {0};
#endif

/* Prepara, fora da medicao, as entradas da operacao medida */
static void setup(void) {
#if defined(SIGN)
    if (ALG_keygen(&sk, &pk)) abort();

#elif defined(VER)
    if (ALG_keygen(&sk, &pk)) abort();
    if (ALG_sign(&sk, message, MSG_LEN, &signature)) abort();

    /* Garante que a assinatura usada na medicao e valida */
    if (ALG_verify(&pk, message, MSG_LEN, &signature)) {
        fprintf(stderr, "ERRO: assinatura de setup nao verifica!\n");
        abort();
    }
#endif
}

/* [ADAPTAR] imprime os parametros e tamanhos informados pela implementacao */
static void info(void) {
    fprintf(stderr, "ALGORITMO benchmarking utility\n");
    fprintf(stderr, "Conjunto de parametros: %s\n", ALG_PARAM_NAME);
    fprintf(stderr, "Chave privada: %zuB\n", (size_t)ALG_SECRETKEYBYTES);
    fprintf(stderr, "Chave publica: %zuB\n", (size_t)ALG_PUBLICKEYBYTES);
    fprintf(stderr, "Assinatura:    %zuB\n", (size_t)ALG_SIGNATUREBYTES);
}

int main(void) {

    /* [ADAPTAR] semente fixa do gerador pseudoaleatorio, se o esquema exigir */
    ALG_csprng_init(/* semente fixa */);

    fprintf(stderr, "===== ALGORITMO BENCHMARK =====\n");
    fprintf(stderr, "BENCH                : %d (%d execucoes internas)\n",
            BENCH, BENCH * BENCH);

    setup();

    /* [ADAPTAR] uma unica chamada ao CLOCKS, com a funcao medida */
#if defined(GEN)
    fprintf(stderr, "Funcao               : KeyGen\n\n");
    CLOCKS("Geracao", ALG_keygen(&sk, &pk));

#elif defined(SIGN)
    fprintf(stderr, "Tamanho da mensagem  : %d bytes\n", MSG_LEN);
    fprintf(stderr, "Funcao               : Sign\n\n");
    CLOCKS("Assinatura", ALG_sign(&sk, message, MSG_LEN, &signature));

#elif defined(VER)
    fprintf(stderr, "Tamanho da mensagem  : %d bytes\n", MSG_LEN);
    fprintf(stderr, "Funcao               : Verify\n\n");
    CLOCKS("Verificacao", ALG_verify(&pk, message, MSG_LEN, &signature));
#endif

    fprintf(stderr, "\n");
    info();

    return 0;
}