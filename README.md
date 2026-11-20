# Benchmarks de assinaturas pós-quânticas

Este repositório reúne o código usado no meu TCC, que compara o desempenho de sete esquemas de assinatura digital pós-quânticos da segunda rodada de assinaturas adicionais do NIST. Os esquemas são SQIsign, CROSS, LESS, UOV, QR-UOV, SNOVA e MAYO. Cada um está em sua própria pasta, com o código da submissão original e um programa de benchmark adaptado para medir uma operação por vez. A explicação da metodologia e os resultados estão no texto do TCC.

## Arquivos da raiz

- `Bench_template.c` é a estrutura comum seguida por todos os benchmarks.
- `defines.h` escolhe a operação medida (`GEN`, `SIGN` ou `VER`).
- `test_setup.sh` prepara o ambiente antes das medições.
- `undo_test_setup.sh` desfaz essa preparação.

## Benchmarks

| Algoritmo | Caminho do benchmark |
|-----------|----------------------|
| SQIsign | `sqisign-submission-round2/Reference_Implementation/apps/sqiSign_Bench.c` |
| CROSS | `CROSS/Additional_Implementations/Benchmarking/lib/cross_bench.c` |
| LESS | `LESS/Submission_LESS_Round2/Utilities/Benchmarking/lib/less_bench.c` |
| UOV | `uov-submission-round2/UOV/Optimized_Implementation/avx2/uov_bench.c` |
| QR-UOV | `qr-uov-submission-round2/QR_UOV - Round2/Alternative_Implementation/qr-uov_bench.c` |
| SNOVA | `snova-submission-round2/SNOVA_Round2/Optimized_Implementation/src/test_speed/snova_bench.c` |
| MAYO | `mayo-submission-round2/MAYO/Optimized_Implementation/test/mayo_bench_my.c` |

## Configurações avaliadas no TCC

| Algoritmo | Categoria 1 | Categoria 3 | Categoria 5 |
|-----------|-------------|-------------|-------------|
| SQIsign | `lvl1` | `lvl3` | `lvl5` |
| CROSS | `cat_1_RSDP_BALANCED` | `cat_3_RSDP_BALANCED` | `cat_5_RSDP_BALANCED` |
| LESS | `cat_252_192` | `cat_400_220` | `cat_548_345` |
| UOV | `Ip_pkc_skc` | `III_pkc_skc` | `V_pkc_skc` |
| QR-UOV | `qruov1q127L3v156m54` | `qruov3q127L3v228m78` | `qruov5q127L3v306m105` |
| SNOVA | V=24, O=5, L=4 | V=37, O=8, L=4 | V=60, O=10, L=4 |
| MAYO | `MAYO_1` | `MAYO_3` | `MAYO_5` |

## Como executar

1. Prepare o ambiente com `sudo ./test_setup.sh`.
2. Escolha a operação em `defines.h`, deixando apenas uma das linhas `#define GEN`, `#define SIGN` ou `#define VER`.
3. Compile o benchmark do algoritmo (comandos abaixo). Compile de novo sempre que mudar `defines.h` e confira na saída do programa a linha `Funcao`, que mostra a operação medida.
4. Execute o binário no núcleo reservado.

```sh
sudo chrt -f 99 taskset -c 1 ./<binario>
```

No QR-UOV, use `OMP_NUM_THREADS=1` para limitar o OpenMP a uma thread.

```sh
sudo OMP_NUM_THREADS=1 chrt -f 99 taskset -c 1 ./qruov_bench
```

Ao terminar, rode `sudo ./undo_test_setup.sh` ou reinicie a máquina.

Os benchmarks e alguns Makefiles referenciam a pasta `/home/sla/Downloads/algorithms`. Para compilar em outro local, ajuste esse caminho.

### Compilação

Os comandos abaixo compilam a configuração da categoria 1 de cada algoritmo. Para as outras categorias, troque o alvo ou os parâmetros conforme a tabela de configurações avaliadas.

**SQIsign**

```sh
cd sqisign-submission-round2/build
cmake -DSQISIGN_BUILD_TYPE=broadwell -DCMAKE_BUILD_TYPE=Release ../Reference_Implementation
make benchmark_lvl1
# binário em apps/benchmark_lvl1
```

**CROSS**

```sh
cd CROSS/Additional_Implementations/Benchmarking/build
cmake ..
make CROSS_benchmark_cat_1_RSDP_BALANCED
# binário em bin/CROSS_benchmark_cat_1_RSDP_BALANCED
```

**LESS** (o AVX2 já vem ativado por padrão)

```sh
cd LESS/Submission_LESS_Round2/Utilities/Benchmarking/build
cmake -DCMAKE_BUILD_TYPE=Release ..
make LESS_benchmark_cat_252_192
# binário em LESS_benchmark_cat_252_192
```

**UOV** (versão com as chaves comprimidas)

```sh
cd uov-submission-round2/UOV/Optimized_Implementation/avx2
make clean && make uov_bench PROJ=Ip_pkc_skc CC=gcc
# binário em uov_bench
```

**QR-UOV** (compila os três conjuntos listados no `Makefile`)

```sh
cd "qr-uov-submission-round2/QR_UOV - Round2/Alternative_Implementation"
make
# binários em qruov1q127L3v156m54/avx2a/qruov_bench, qruov3q127L3v228m78/avx2a/qruov_bench e qruov5q127L3v306m105/avx2a/qruov_bench
```

**SNOVA** (chave privada em forma de semente e expansão por AES)

```sh
cd snova-submission-round2/SNOVA_Round2/Optimized_Implementation/src
make clean_without_libsnovasym snova_bench SNOVA_V=24 SNOVA_O=5 SNOVA_L=4 SK_IS_SEED=1 PK_EXPAND_SHAKE=0 OPTIMISATION=2
# binário em snova_bench.a
```

**MAYO**

```sh
cd mayo-submission-round2/MAYO/Optimized_Implementation/build
cmake -DMAYO_BUILD_TYPE=avx2 -DENABLE_AESNI=ON -DENABLE_STRICT=OFF ..
make mayo_bench_MAYO_1
# binário em test/mayo_bench_MAYO_1
```