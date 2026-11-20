
#include <stdio.h>
#include <stdlib.h>

#include <mayo.h>

#include "/home/sla/Downloads/algorithms/defines.h"
#include "clocks.h"


#if (defined(GEN) + defined(SIGN) + defined(VER)) != 1
#error "defines.h: ative EXATAMENTE UM de GEN, SIGN ou VER"
#endif

#define MSG_LEN 8

#define PARAMS NULL

static unsigned char *pk;
static unsigned char *sk;

#if defined(SIGN) || defined(VER)
static unsigned char *sm;                 
static unsigned char  m[MSG_LEN] = {0};   
static size_t         smlen;              
#endif
#if defined(VER)
static size_t mlen_out;                 
#endif


static void setup(void) {
    pk = calloc(PARAM_cpk_bytes(PARAMS), 1);
    sk = calloc(PARAM_csk_bytes(PARAMS), 1);
    if (!pk || !sk) abort();

#if defined(GEN)
#define BENCH 1000

#elif defined(SIGN)
#define BENCH 600
    sm = calloc(PARAM_sig_bytes(PARAMS) + MSG_LEN, 1);
    if (!sm) abort();
    if (mayo_keypair(PARAMS, pk, sk)) abort();
    smlen = PARAM_sig_bytes(PARAMS) + MSG_LEN;  

#elif defined(VER)
#define BENCH 900
    sm = calloc(PARAM_sig_bytes(PARAMS) + MSG_LEN, 1);
    if (!sm) abort();
    if (mayo_keypair(PARAMS, pk, sk)) abort();

    smlen = PARAM_sig_bytes(PARAMS) + MSG_LEN;
    if (mayo_sign(PARAMS, sm, &smlen, m, MSG_LEN, sk)) abort();

    mlen_out = MSG_LEN;
    if (mayo_open(PARAMS, m, &mlen_out, sm, smlen, pk)) {
        fprintf(stderr, "ERRO: assinatura de setup nao verifica!\n");
        abort();
    }
#endif
}


static void info(void) {
    fprintf(stderr, "MAYO benchmarking utility\n");
    fprintf(stderr, "Parameter set: %s\n", PARAM_name(PARAMS));
    fprintf(stderr, "Public key : %zuB\n", (size_t)PARAM_cpk_bytes(PARAMS));
    fprintf(stderr, "Private key: %zuB\n", (size_t)PARAM_csk_bytes(PARAMS));
    fprintf(stderr, "Signature  : %zuB (%.2f KB)\n",
            (size_t)PARAM_sig_bytes(PARAMS),
            (float)PARAM_sig_bytes(PARAMS) / 1024.0f);
}

int main(void) {


    fprintf(stderr, "===== MAYO BENCHMARK =====\n");
    fprintf(stderr, "BENCH                : %d (%d execucoes internas)\n",
            BENCH, BENCH * BENCH);


    setup();

#if defined(GEN)
    fprintf(stderr, "Funcao               : KeyGen (mayo_keypair)\n\n");
    CLOCKS("Geracao", mayo_keypair(PARAMS, pk, sk));

#elif defined(SIGN)
    fprintf(stderr, "Tamanho da mensagem  : %d bytes\n", MSG_LEN);
    fprintf(stderr, "Funcao               : Sign (mayo_sign)\n\n");
    CLOCKS("Assinatura", mayo_sign(PARAMS, sm, &smlen, m, MSG_LEN, sk));

#elif defined(VER)
    fprintf(stderr, "Tamanho da mensagem  : %d bytes\n", MSG_LEN);
    fprintf(stderr, "Funcao               : Verify (mayo_open)\n\n");
    CLOCKS("Verificacao", mayo_open(PARAMS, m, &mlen_out, sm, smlen, pk));
#endif

    fprintf(stderr, "\n");
    info();

    return 0;
}