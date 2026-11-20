#include <stdio.h>
#include <stdlib.h>

#include "CROSS.h"
#include "csprng_hash.h"

#include "/home/sla/Downloads/algorithms/defines.h"
#include "clocks.h"

#if (defined(GEN) + defined(SIGN) + defined(VER)) != 1
#error "defines.h: ative EXATAMENTE UM de GEN, SIGN ou VER"
#endif

#if defined(GEN)
#define BENCH 2200
#elif defined(SIGN)
#define BENCH 300
#elif defined(VER)
#define BENCH 380
#endif

#define MSG_LEN 8

static pk_t pk;
static sk_t sk;

#if defined(SIGN) || defined(VER)
static CROSS_sig_t signature;
static char message[MSG_LEN] = "Signme!!";
#endif

static void setup(void) {
#if defined(SIGN)
    CROSS_keygen(&sk, &pk);

#elif defined(VER)
    CROSS_keygen(&sk, &pk);
    CROSS_sign(&sk, message, MSG_LEN, &signature);

    if (CROSS_verify(&pk, message, MSG_LEN, &signature) != 1) {
        fprintf(stderr, "ERRO: assinatura de setup nao verifica!\n");
        abort();
    }
#endif
}

static void info(void) {
    fprintf(stderr, "CROSS benchmarking utility\n");

#if defined(RSDP)
    fprintf(stderr, "Problem: RSDP\n");
#elif defined(RSDPG)
    fprintf(stderr, "Problem: RSDP(G)\n");
#endif

#if defined(CATEGORY_1)
    fprintf(stderr, "Category: 1\n");
#elif defined(CATEGORY_3)
    fprintf(stderr, "Category: 3\n");
#elif defined(CATEGORY_5)
    fprintf(stderr, "Category: 5\n");
#endif

#if defined(SIG_SIZE)
    fprintf(stderr, "Optimization: SIG_SIZE\n");
#elif defined(BALANCED)
    fprintf(stderr, "Optimization: BALANCED\n");
#elif defined(SPEED)
    fprintf(stderr, "Optimization: SPEED\n");
#endif

    fprintf(stderr, "Code parameters: n=%d, k=%d, p=%d\n", N, K, P);
    fprintf(stderr, "Restriction size: z=%d\n", Z);
    fprintf(stderr, "Fixed weight challenge: %d rounds, weight %d\n", T, W);
    fprintf(stderr, "Private key: %luB\n", sizeof(sk_t));
    fprintf(stderr, "Public key:  %luB\n", sizeof(pk_t));
    fprintf(stderr, "Signature:   %luB\n", sizeof(CROSS_sig_t));
}

int main(void) {

    csprng_initialize(&platform_csprng_state,
                      (const unsigned char *)"0123456789012345", 16, 0);

    fprintf(stderr, "===== CROSS BENCHMARK =====\n");
    fprintf(stderr, "BENCH                : %d (%d execucoes internas)\n",
            BENCH, BENCH * BENCH);

    setup();

#if defined(GEN)
    fprintf(stderr, "Funcao               : KeyGen (CROSS_keygen)\n\n");
    CLOCKS("Geracao", CROSS_keygen(&sk, &pk));

#elif defined(SIGN)
    fprintf(stderr, "Tamanho da mensagem  : %d bytes\n", MSG_LEN);
    fprintf(stderr, "Funcao               : Sign (CROSS_sign)\n\n");
    CLOCKS("Assinatura", CROSS_sign(&sk, message, MSG_LEN, &signature));

#elif defined(VER)
    fprintf(stderr, "Tamanho da mensagem  : %d bytes\n", MSG_LEN);
    fprintf(stderr, "Funcao               : Verify (CROSS_verify)\n\n");
    CLOCKS("Verificacao", CROSS_verify(&pk, message, MSG_LEN, &signature));
#endif

    fprintf(stderr, "\n");
    info();

    return 0;
}