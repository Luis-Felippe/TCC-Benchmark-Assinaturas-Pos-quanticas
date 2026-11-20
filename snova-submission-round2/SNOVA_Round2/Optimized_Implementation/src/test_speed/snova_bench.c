

#include <stdio.h>
#include <stdlib.h>

#define NO_MT4B
#include "../snova.h"
#include "../util/util.h"
#include "../nistkat/rng.h"

#include "/home/sla/Downloads/algorithms/defines.h"
#include "clocks.h"


#if (defined(GEN) + defined(SIGN) + defined(VER)) != 1
#error "defines.h: ative EXATAMENTE UM de GEN, SIGN ou VER"
#endif


#define DIGEST_LEN 64


static uint8_t pk[bytes_pk];
static uint8_t ssk[seed_length];
static uint8_t seed[seed_length];
static uint8_t *pt_public_key_seed;
static uint8_t *pt_private_key_seed;

#if defined(SIGN) || defined(VER)
static uint8_t digest[DIGEST_LEN] = {0};
static uint8_t salt[bytes_salt];
static uint8_t signature[bytes_signature + bytes_salt];
#endif


static void setup(void) {
    uint8_t entropy_input[48];
    for (int i = 0; i < 48; i++) {
        entropy_input[i] = (uint8_t)i;
    }
    randombytes_init(entropy_input, NULL, 256);
    randombytes(seed, seed_length);

    pt_public_key_seed  = seed;
    pt_private_key_seed = seed + seed_length_public;

#if defined(GEN)
#define BENCH 500

#elif defined(SIGN)
#define BENCH 400
    randombytes(salt, bytes_salt);
    generate_keys_ssk(pk, ssk, pt_public_key_seed, pt_private_key_seed);

#elif defined(VER)
#define BENCH 600
    randombytes(salt, bytes_salt);
    generate_keys_ssk(pk, ssk, pt_public_key_seed, pt_private_key_seed);
    sign_digest_ssk(signature, digest, DIGEST_LEN, salt, ssk);

    if (verify_signture(digest, DIGEST_LEN, signature, pk) != 0) {
        fprintf(stderr, "ERRO: assinatura de setup nao verifica!\n");
        abort();
    }
#endif
}


static void info(void) {
    fprintf(stderr, "SNOVA benchmarking utility\n");
    fprintf(stderr, "Algoritmo   : %s\n", CRYPTO_ALGNAME);
    fprintf(stderr, "(V, O, L)   : (%d, %d, %d)\n", v_SNOVA, o_SNOVA, l_SNOVA);
#if PK_EXPAND_SHAKE
    fprintf(stderr, "PK_EXPAND   : SHAKE\n");
#else
    fprintf(stderr, "PK_EXPAND   : AES\n");
#endif
    fprintf(stderr, "Modo de sk  : ssk (semente)\n");
    fprintf(stderr, "Public key  : %dB\n", bytes_pk);
    fprintf(stderr, "Private key : %dB\n", seed_length);
    fprintf(stderr, "Assinatura  : %dB (%.2f KB)\n",
            bytes_signature + bytes_salt,
            (float)(bytes_signature + bytes_salt) / 1024.0f);
}

int main(void) {
    snova_init();

    fprintf(stderr, "===== SNOVA BENCHMARK =====\n");
    fprintf(stderr, "BENCH                : %d (%d execucoes internas)\n",
            BENCH, BENCH * BENCH);


    setup();

#if defined(GEN)
    fprintf(stderr, "Funcao               : KeyGen (generate_keys_ssk)\n\n");
    CLOCKS("Geracao",
           generate_keys_ssk(pk, ssk, pt_public_key_seed, pt_private_key_seed));

#elif defined(SIGN)
    fprintf(stderr, "Tamanho do digest    : %d bytes\n", DIGEST_LEN);
    fprintf(stderr, "Funcao               : Sign (sign_digest_ssk)\n\n");
    CLOCKS("Assinatura",
           sign_digest_ssk(signature, digest, DIGEST_LEN, salt, ssk));

#elif defined(VER)
    fprintf(stderr, "Tamanho do digest    : %d bytes\n", DIGEST_LEN);
    fprintf(stderr, "Funcao               : Verify (verify_signture)\n\n");
    CLOCKS("Verificacao",
           verify_signture(digest, DIGEST_LEN, signature, pk));
#endif

    fprintf(stderr, "\n");
    info();

    return 0;
}