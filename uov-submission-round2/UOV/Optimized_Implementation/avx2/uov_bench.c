
#include <stdio.h>
#include <stdlib.h>

#include "api.h"

#include "/home/sla/Downloads/algorithms/defines.h"
#include "clocks.h"

#if (defined(GEN) + defined(SIGN) + defined(VER)) != 1
#error "defines.h: ative EXATAMENTE UM de GEN, SIGN ou VER"
#endif

#define MSG_LEN 8

#define SM_LEN (CRYPTO_BYTES + MSG_LEN)

static unsigned char *pk;
static unsigned char *sk;

#if defined(SIGN) || defined(VER)
static unsigned char sm[SM_LEN];
static unsigned char m[MSG_LEN] = {0};  
static unsigned long long smlen;
#endif
#if defined(VER)
static unsigned long long mlen_out;
#endif

static void setup(void) {
    pk = (unsigned char *)malloc(CRYPTO_PUBLICKEYBYTES);
    sk = (unsigned char *)malloc(CRYPTO_SECRETKEYBYTES);
    if (!pk || !sk) {
        fprintf(stderr, "ERRO: falha ao alocar as chaves\n");
        abort();
    }

#if defined(GEN)
    #define BENCH 190

#elif defined(SIGN)
    #define BENCH 250
    if (crypto_sign_keypair(pk, sk)) abort();
    smlen = SM_LEN;

#elif defined(VER)
    #define BENCH 900
    if (crypto_sign_keypair(pk, sk)) abort();

    smlen = SM_LEN;
    if (crypto_sign(sm, &smlen, m, MSG_LEN, sk)) abort();

    mlen_out = MSG_LEN;
    if (crypto_sign_open(m, &mlen_out, sm, smlen, pk)) {
        fprintf(stderr, "ERRO: assinatura de setup nao verifica!\n");
        abort();
    }
#endif
}

static void info(void) {
    fprintf(stderr, "UOV benchmarking utility\n");
    fprintf(stderr, "Algoritmo   : %s\n", CRYPTO_ALGNAME);
    fprintf(stderr, "Public key  : %zuB (%.2f KB)\n",
            (size_t)CRYPTO_PUBLICKEYBYTES,
            (float)CRYPTO_PUBLICKEYBYTES / 1024.0f);
    fprintf(stderr, "Private key : %zuB\n", (size_t)CRYPTO_SECRETKEYBYTES);
    fprintf(stderr, "Assinatura  : %zuB\n", (size_t)CRYPTO_BYTES);
}

int main(void) {

    fprintf(stderr, "===== UOV BENCHMARK =====\n");
    fprintf(stderr, "BENCH                : %d (%d execucoes internas)\n",
            BENCH, BENCH * BENCH);

    setup();

#if defined(GEN)
    fprintf(stderr, "Funcao               : KeyGen (crypto_sign_keypair)\n\n");
    CLOCKS("Geracao", crypto_sign_keypair(pk, sk));

#elif defined(SIGN)
    fprintf(stderr, "Tamanho da mensagem  : %d bytes\n", MSG_LEN);
    fprintf(stderr, "Funcao               : Sign (crypto_sign)\n\n");
    CLOCKS("Assinatura", crypto_sign(sm, &smlen, m, MSG_LEN, sk));

#elif defined(VER)
    fprintf(stderr, "Tamanho da mensagem  : %d bytes\n", MSG_LEN);
    fprintf(stderr, "Funcao               : Verify (crypto_sign_open)\n\n");
    CLOCKS("Verificacao", crypto_sign_open(m, &mlen_out, sm, SM_LEN, pk));
#endif

    fprintf(stderr, "\n");
    info();

    free(pk);
    free(sk);

    return 0;
}