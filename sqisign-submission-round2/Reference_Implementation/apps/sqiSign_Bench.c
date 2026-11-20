
#include <stdio.h>
#include <stdlib.h>

#include <api.h>
#include <rng.h>

#include "/home/sla/Downloads/algorithms/defines.h"
#include "clocks.h"


#if (defined(GEN) + defined(SIGN) + defined(VER)) != 1
#error "defines.h: ative EXATAMENTE UM de GEN, SIGN ou VER"
#endif


#define MSG_LEN 8

#define SM_LEN (CRYPTO_BYTES + MSG_LEN)


static unsigned char pk[CRYPTO_PUBLICKEYBYTES];
static unsigned char sk[CRYPTO_SECRETKEYBYTES];

#if defined(SIGN) || defined(VER)
static unsigned char sm[SM_LEN];
static unsigned char m[MSG_LEN];
static unsigned long long smlen;
#endif

static void setup(void) {
#if defined(GEN)
#define BENCH 65

#elif defined(SIGN)
#define BENCH 40
    if (randombytes(m, MSG_LEN))        abort();
    if (crypto_sign_keypair(pk, sk))    abort();

#elif defined(VER)
#define BENCH 160
    if (randombytes(m, MSG_LEN))        abort();
    if (crypto_sign_keypair(pk, sk))    abort();

    smlen = SM_LEN;
    if (crypto_sign(sm, &smlen, m, MSG_LEN, sk)) abort();
    if (smlen != SM_LEN)                abort();

    unsigned long long mlen_out = MSG_LEN;
    if (crypto_sign_open(m, &mlen_out, sm, SM_LEN, pk)) {
        fprintf(stderr, "ERRO: assinatura de setup nao verifica!\n");
        abort();
    }
#endif
}

static void info(void) {
    fprintf(stderr, "SQIsign benchmarking utility\n");
    fprintf(stderr, "Algoritmo   : %s\n", CRYPTO_ALGNAME);
    fprintf(stderr, "Public key  : %zuB\n", (size_t)CRYPTO_PUBLICKEYBYTES);
    fprintf(stderr, "Private key : %zuB\n", (size_t)CRYPTO_SECRETKEYBYTES);
    fprintf(stderr, "Assinatura  : %zuB (%.2f KB)\n",
            (size_t)CRYPTO_BYTES, (float)CRYPTO_BYTES / 1024.0f);
}

int main(void) {

    unsigned char seed[48];
    for (int i = 0; i < 48; i++) {
        seed[i] = (unsigned char)i;
    }
    randombytes_init(seed, NULL, 256);

    fprintf(stderr, "===== SQISIGN BENCHMARK =====\n");
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
    CLOCKS("Verificacao", crypto_sign_open(m, &smlen, sm, SM_LEN, pk));
#endif

    fprintf(stderr, "\n");
    info();

    return 0;
}