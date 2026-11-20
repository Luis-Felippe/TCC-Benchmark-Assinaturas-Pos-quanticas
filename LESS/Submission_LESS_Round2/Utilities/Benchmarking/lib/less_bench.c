
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "LESS.h"
#include "codes.h"
#include "cycles.h"
#include "monomial_mat.h"
#include "rng.h"
#include "test_helpers.h"
#include "api.h"

#include "/home/sla/Downloads/algorithms/defines.h"
#include "clocks.h"


#if (defined(GEN) + defined(SIGN) + defined(VER)) != 1
#error "defines.h: ative EXATAMENTE UM de GEN, SIGN ou VER"
#endif

#define MSG_LEN 8

static pubkey_t pk;
static prikey_t sk;

#if defined(SIGN) || defined(VER)
static sign_t signature;
static char message[MSG_LEN] = "Signme!!";
#endif

static void setup(void) {
#if defined(GEN)
#define BENCH 400

#elif defined(SIGN)
#define BENCH 20
    LESS_keygen(&sk, &pk);

#elif defined(VER)
#define BENCH 20
    LESS_keygen(&sk, &pk);
    LESS_sign(&sk, message, MSG_LEN, &signature);
#endif
}


static void info(void) {
    fprintf(stderr, "LESS benchmarking utility\n");
    fprintf(stderr, "Code parameters: n=%d, k=%d, q=%d\n", N, K, Q);
    fprintf(stderr, "Num. keypairs: %d\n", NUM_KEYPAIRS);
    fprintf(stderr, "Fixed weight challenge: %d rounds, weight %d\n", T, W);
    fprintf(stderr, "Private key: %luB\n", sizeof(prikey_t));
    fprintf(stderr, "Public key:  %luB\n", sizeof(pubkey_t));
    fprintf(stderr, "Signature:   %luB (%.2f KB)\n",
            sizeof(sign_t), (float)sizeof(sign_t) / 1024.0f);
}

int main(void) {


    setup_cycle_counter();


    initialize_csprng(&platform_csprng_state,
                      (const unsigned char *)"0123456789012345", 16);

    fprintf(stderr, "===== LESS BENCHMARK =====\n");
    fprintf(stderr, "BENCH                : %d (%d execucoes internas)\n",
            BENCH, BENCH * BENCH);

    setup();


#if defined(GEN)
    fprintf(stderr, "Funcao               : KeyGen (LESS_keygen)\n\n");
    CLOCKS("Geracao", LESS_keygen(&sk, &pk));

#elif defined(SIGN)
    fprintf(stderr, "Tamanho da mensagem  : %d bytes\n", MSG_LEN);
    fprintf(stderr, "Funcao               : Sign (LESS_sign)\n\n");
    CLOCKS("Assinatura", LESS_sign(&sk, message, MSG_LEN, &signature));

#elif defined(VER)
    fprintf(stderr, "Tamanho da mensagem  : %d bytes\n", MSG_LEN);
    fprintf(stderr, "Funcao               : Verify (LESS_verify)\n\n");
    CLOCKS("Verificacao", LESS_verify(&pk, message, MSG_LEN, &signature));
#endif

    fprintf(stderr, "\n");
    info();

    return 0;
}