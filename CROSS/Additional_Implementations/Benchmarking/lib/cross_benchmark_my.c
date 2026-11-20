#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define HIGH_PERFORMANCE_X86_64
#include "timing_and_stat.h"
#include "CROSS.h"
#include "csprng_hash.h"

#include "/home/sla/Downloads/algorithms/defines.h"
#include "clocks.h"


#define NUM_TESTS 10

void microbench(){
    welford_t timer;
    welford_init(&timer);

    uint64_t cycles;
    for(int i = 0; i <NUM_TESTS; i++) {
        cycles = x86_64_rtdsc();
        welford_update(&timer,(x86_64_rtdsc()-cycles)/1000.0);
    }
    fprintf(stderr,"microbench kCycles (avg,stddev):");
    welford_print(timer);
    printf("\n");

}

void info(){
    fprintf(stderr,"CROSS benchmarking utility\n");
    fprintf(stderr,"Code parameters: n= %d, k= %d, p=%d\n", N,K,P);
    fprintf(stderr,"restriction size: z=%d\n",Z);
    fprintf(stderr,"Fixed weight challenge vector: %d rounds, weight %d \n",T,W);
    fprintf(stderr,"Private key: %luB\n", sizeof(sk_t));
    fprintf(stderr,"Public key %luB\n", sizeof(pk_t));
    fprintf(stderr,"Signature: %luB\n", sizeof(CROSS_sig_t));

}

void CROSS_sign_verify_speed(int print_tex){
    fprintf(stderr,"Computing number of clock cycles as the average of %d runs\n", NUM_TESTS);
    pk_t pk;
    sk_t sk;
    
    welford_t timer_KG,timer_Sig,timer_Ver;
    welford_init(&timer_KG);
    welford_init(&timer_Sig);
    welford_init(&timer_Ver);
    
    #ifdef GEN
    
    
    #elif defined(SIGN) || defined(VER)
    CROSS_sig_t signature;
//     char message[1] = "S";
      char message[8] = "Signme!!";
//     char message[32] = "Signme!!Signme!!Signme!!Signme!";
      // char message[128] = "Signme!!Signme!!Signme!!Signme!Signme!!Signme!!Signme!!Signme!Signme!!Signme!!Signme!!Signme!Signme!!Signme!!Signme!!Signme!";
      // char message[512] = "Signme!!Signme!!Signme!!Signme!Signme!!Signme!!Signme!!Signme!Signme!!Signme!!Signme!!Signme!Signme!!Signme!!Signme!!Signme!Signme!!Signme!!Signme!!Signme!Signme!!Signme!!Signme!!Signme!Signme!!Signme!!Signme!!Signme!Signme!!Signme!!Signme!!Signme!Signme!!Signme!!Signme!!Signme!Signme!!Signme!!Signme!!Signme!Signme!!Signme!!Signme!!Signme!Signme!!Signme!!Signme!!Signme!Signme!!Signme!!Signme!!Signme!Signme!!Signme!!Signme!!Signme!Signme!!Signme!!Signme!!Signme!Signme!!Signme!!Signme!!Signme!";
    #endif

    #ifdef GEN
    #define BENCH 50

    for(int i = 0; i <NUM_TESTS; i++) {
        CLOCKS("Geração" ,CROSS_keygen(&sk,&pk));
    }

    #elif defined(SIGN)
    #define BENCH 50

    for(int i = 0; i <NUM_TESTS; i++) {
        CROSS_keygen(&sk,&pk);
        CLOCKS("Assinatura" ,CROSS_sign(&sk,message,32,&signature));
    }

    #elif defined(VER)
    #define BENCH 50

    for(int i = 0; i <NUM_TESTS; i++) {
      CROSS_keygen(&sk,&pk);
      CROSS_sign(&sk,message,32,&signature);
        CLOCKS("Verificação", CROSS_verify(&pk,message,32,&signature));
    }


    #endif

    if(print_tex){
      /* print a convenient machine extractable table row pair */
      printf("TIME & ");
#if defined(RSDP)
      printf("RSDP    & ");
#elif defined(RSDPG)
      printf("RSDP(G) & ");
#endif
#if defined(SHA2_HASH)
      printf("SHA-2 & ");
#elif defined(SHA3_HASH)
      printf("SHA-3 & ");
#endif
#if defined(CATEGORY_1)
      printf("Cat. 1 & ");
#elif defined(CATEGORY_3)
      printf("Cat. 3 & ");
#elif defined(CATEGORY_5)
      printf("Cat. 5 & ");
#endif
#if defined(SIG_SIZE)
      printf("Size  & ");
#elif defined(BALANCED)
      printf("Balan & ");      
#elif defined(SPEED)
      printf("Speed & ");
#endif
      // printf(" & ");
      welford_print_tex(timer_KG);
      printf(" & ");
      welford_print_tex(timer_Sig);
      printf(" & ");
      welford_print_tex(timer_Ver);
      printf("\n SPACE & ");
#if defined(RSDP)
      printf("RSDP    & ");
#elif defined(RSDPG)
      printf("RSDP(G) & ");
#endif
#if defined(CATEGORY_1)
      printf("Cat. 1 & ");
#elif defined(CATEGORY_3)
      printf("Cat. 3 & ");
#elif defined(CATEGORY_5)
      printf("Cat. 5 & ");
#endif
#if defined(SIG_SIZE)
      printf("Size  & ");
#elif defined(SPEED)
      printf("Speed & ");
#endif
      printf(" %lu &", sizeof(sk_t));
      printf(" %lu &", sizeof(pk_t));
      printf(" %lu ", sizeof(CROSS_sig_t));
      printf(" \\\\\n");
    } else {
        info();
    }
}


int main(int argc, char* argv[]){
    csprng_initialize(&platform_csprng_state,
                      (const unsigned char *)"0123456789012345",16,0);
    fprintf(stderr,"CROSS referenced implementation benchmarking tool\n");
    if ( (argc>1) &&
         (argv[1][0] == '-' ) &&
         (argv[1][1] == 'T' )){
        CROSS_sign_verify_speed(1);
    } else {
        CROSS_sign_verify_speed(0);
    }
    return 0;
}