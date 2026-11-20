#!/bin/bash

# Prepara o ambiente para as medicoes de benchmark (Ubuntu, CPU AMD Ryzen).
# Uso: sudo ./test_setup.sh
# Todas as alteracoes sao temporarias e revertidas ao reiniciar o sistema.

TARGET_GOVERNOR="performance"

# Nucleo reservado para o benchmark
BENCH_CORE=1

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_section() {
    echo
    echo -e "${BLUE}====================================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}====================================================${NC}"
}

print_ok() {
    echo -e "${GREEN}[OK]${NC} $1"
}

print_warn() {
    echo -e "${YELLOW}[!]${NC} $1"
}

print_err() {
    echo -e "${RED}[X]${NC} $1"
}

if [ "$EUID" -ne 0 ]; then
    print_err "Execute como root:"
    echo "sudo $0"
    exit 1
fi

print_section "INFORMAÇÕES DO SISTEMA"

source /etc/os-release

print_ok "Distribuição: $PRETTY_NAME"

CPU_MODEL=$(grep "model name" /proc/cpuinfo | head -1 | cut -d ':' -f2 | xargs)
print_ok "CPU: $CPU_MODEL"

# 1. Servicos de gerenciamento de energia
print_section "1. Desabilitando serviços de gerenciamento de energia"

for service in \
    power-profiles-daemon \
    tuned
do
    if systemctl is-active --quiet "$service" 2>/dev/null; then
        systemctl stop "$service"
        print_ok "$service parado"
    else
        print_warn "$service não está ativo"
    fi
done

# 2. Tarefas automaticas em segundo plano
print_section "2. Parando serviços de background"

for service in \
    apt-daily.service \
    apt-daily-upgrade.service \
    unattended-upgrades \
    packagekit \
    tracker-miner-fs-3 \
    tracker-extract-3
do
    if systemctl is-active --quiet "$service" 2>/dev/null; then
        systemctl stop "$service"
        print_ok "$service parado"
    else
        print_warn "$service não está ativo"
    fi
done

# 3. SMT
print_section "3. Desativando SMT"

if [ -e /sys/devices/system/cpu/smt/control ]; then

    echo off > /sys/devices/system/cpu/smt/control

    SMT_STATUS=$(cat /sys/devices/system/cpu/smt/control)

    print_ok "SMT: $SMT_STATUS"

else
    print_warn "Controle SMT não encontrado"
fi

# 4. Desliga todos os nucleos, exceto o CPU0 e o do benchmark
print_section "4. Mantendo apenas CPU0 e CPU${BENCH_CORE}"

for cpu_path in /sys/devices/system/cpu/cpu[0-9]*; do

    cpu_num=$(basename "$cpu_path" | sed 's/cpu//')

    if [ "$cpu_num" = "0" ] || [ "$cpu_num" = "$BENCH_CORE" ]; then
        continue
    fi

    if [ -e "$cpu_path/online" ]; then

        current=$(cat "$cpu_path/online")

        if [ "$current" = "1" ]; then

            echo 0 > "$cpu_path/online" 2>/dev/null

            print_ok "CPU${cpu_num} desligada"
        fi
    fi
done

print_ok "CPUs online: $(cat /sys/devices/system/cpu/online)"

# 5. Governor
print_section "5. Configurando governor"

for governor in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do

    [ -e "$governor" ] || continue

    echo "$TARGET_GOVERNOR" > "$governor" 2>/dev/null
done

print_ok "Governor configurado para: $TARGET_GOVERNOR"

# 6. Apenas exibe o estado do driver AMD P-State
print_section "6. Configuração AMD P-State"

if [ -d /sys/devices/system/cpu/amd_pstate ]; then

    print_ok "AMD P-State detectado"

    if [ -f /sys/devices/system/cpu/amd_pstate/status ]; then
        print_ok "Status: $(cat /sys/devices/system/cpu/amd_pstate/status)"
    fi

else
    print_warn "AMD P-State não encontrado"
fi

# 7. C-States
print_section "7. Desativando C-States"

COUNT=0

for state in /sys/devices/system/cpu/cpu*/cpuidle/state*/disable; do

    [ -e "$state" ] || continue

    echo 1 > "$state"

    COUNT=$((COUNT+1))
done

print_ok "$COUNT estados desativados"

# 8. Transparent Huge Pages
print_section "8. Transparent Huge Pages"

if [ -e /sys/kernel/mm/transparent_hugepage/enabled ]; then

    echo never > /sys/kernel/mm/transparent_hugepage/enabled
    echo never > /sys/kernel/mm/transparent_hugepage/defrag

    print_ok "THP desativado"

else

    print_warn "THP não encontrado"
fi

# 9. ASLR
print_section "9. Desativando ASLR"

echo 0 > /proc/sys/kernel/randomize_va_space

print_ok "ASLR: $(cat /proc/sys/kernel/randomize_va_space)"

# 10. Swap
print_section "10. Desativando Swap"

if swapon --show | grep -q .; then

    swapoff -a

    print_ok "Swap desativado"

else

    print_warn "Nenhum swap ativo"
fi

# 11. Direciona as IRQs para o CPU0
print_section "11. Movendo IRQs para CPU0"

MOVED=0

for irq in /proc/irq/*/smp_affinity; do

    if echo 1 > "$irq" 2>/dev/null; then
        MOVED=$((MOVED+1))
    fi
done

print_ok "$MOVED IRQs movidas"

# 12. Limpa os caches do sistema
print_section "12. Limpando caches"

sync

echo 3 > /proc/sys/vm/drop_caches

print_ok "Caches liberados"

print_section "STATUS FINAL"

echo
echo "CPU Online: $(cat /sys/devices/system/cpu/online)"
echo "SMT: $(cat /sys/devices/system/cpu/smt/control 2>/dev/null)"
echo "ASLR: $(cat /proc/sys/kernel/randomize_va_space)"
echo "Governor CPU${BENCH_CORE}: $(cat /sys/devices/system/cpu/cpu${BENCH_CORE}/cpufreq/scaling_governor 2>/dev/null)"
echo "Freq Atual CPU${BENCH_CORE}: $(cat /sys/devices/system/cpu/cpu${BENCH_CORE}/cpufreq/scaling_cur_freq 2>/dev/null) kHz"

echo
echo -e "${GREEN}====================================================${NC}"
echo -e "${GREEN}AMBIENTE PRONTO PARA BENCHMARK${NC}"
echo -e "${GREEN}====================================================${NC}"

echo
echo "Executar benchmark:"
echo
echo "sudo chrt -f 99 taskset -c ${BENCH_CORE} ./seu_benchmark"
echo
echo "Monitorar frequência:"
echo
echo "watch -n 1 \"cat /sys/devices/system/cpu/cpu${BENCH_CORE}/cpufreq/scaling_cur_freq\""
echo
echo "Reverter tudo:"
echo
echo "sudo reboot"
echo