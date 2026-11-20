#!/bin/bash

# =============================================================================
# Restore Script
# Ubuntu 22.04 / 24.04
# AMD Ryzen
#
# Reverte alterações feitas pelo stabilize_bench.sh
#
# Execute:
# sudo ./restore_bench.sh
# =============================================================================

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

# -----------------------------------------------------------------------------
# Root
# -----------------------------------------------------------------------------

if [ "$EUID" -ne 0 ]; then
    print_err "Execute como root:"
    echo "sudo $0"
    exit 1
fi

print_section "INFORMAÇÕES"

source /etc/os-release

print_ok "$PRETTY_NAME"

# -----------------------------------------------------------------------------
# 1. Religar CPUs
# -----------------------------------------------------------------------------

print_section "1. Religando todos os núcleos"

RESTORED=0

for cpu_path in /sys/devices/system/cpu/cpu[0-9]*; do

    cpu_num=$(basename "$cpu_path" | sed 's/cpu//')

    [ "$cpu_num" = "0" ] && continue

    if [ -f "$cpu_path/online" ]; then

        current=$(cat "$cpu_path/online")

        if [ "$current" = "0" ]; then

            echo 1 > "$cpu_path/online"

            RESTORED=$((RESTORED+1))
        fi
    fi
done

print_ok "$RESTORED CPUs religadas"

# -----------------------------------------------------------------------------
# 2. Reativar SMT
# -----------------------------------------------------------------------------

print_section "2. Reativando SMT"

if [ -f /sys/devices/system/cpu/smt/control ]; then

    echo on > /sys/devices/system/cpu/smt/control

    print_ok "SMT: $(cat /sys/devices/system/cpu/smt/control)"

else

    print_warn "Controle SMT não encontrado"
fi

# -----------------------------------------------------------------------------
# 3. Restaurar governor
# -----------------------------------------------------------------------------

print_section "3. Restaurando governor"

RESTORED=0

for governor in /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor; do

    [ -f "$governor" ] || continue

    if echo schedutil > "$governor" 2>/dev/null; then

        RESTORED=$((RESTORED+1))
    fi
done

print_ok "$RESTORED governors restaurados"

# -----------------------------------------------------------------------------
# 4. Reativar C-States
# -----------------------------------------------------------------------------

print_section "4. Reativando C-States"

COUNT=0

for state in /sys/devices/system/cpu/cpu*/cpuidle/state*/disable; do

    [ -f "$state" ] || continue

    echo 0 > "$state"

    COUNT=$((COUNT+1))
done

print_ok "$COUNT estados reativados"

# -----------------------------------------------------------------------------
# 5. Reativar THP
# -----------------------------------------------------------------------------

print_section "5. Reativando Transparent Huge Pages"

if [ -f /sys/kernel/mm/transparent_hugepage/enabled ]; then

    echo madvise > /sys/kernel/mm/transparent_hugepage/enabled
    echo madvise > /sys/kernel/mm/transparent_hugepage/defrag

    print_ok "THP restaurado"

else

    print_warn "THP não encontrado"
fi

# -----------------------------------------------------------------------------
# 6. Reativar ASLR
# -----------------------------------------------------------------------------

print_section "6. Reativando ASLR"

echo 2 > /proc/sys/kernel/randomize_va_space

print_ok "ASLR restaurado"

# -----------------------------------------------------------------------------
# 7. Reativar Swap
# -----------------------------------------------------------------------------

print_section "7. Reativando Swap"

if swapon -a; then

    print_ok "Swap reativado"

else

    print_warn "Nenhum swap configurado"
fi

# -----------------------------------------------------------------------------
# 8. Restaurar IRQs
# -----------------------------------------------------------------------------

print_section "8. Restaurando afinidade de IRQs"

NUM_CPUS=$(nproc)

MASK=$(printf "%x" $(( (1 << NUM_CPUS) - 1 )))

OK=0

for irq in /proc/irq/*/smp_affinity; do

    [ -f "$irq" ] || continue

    if echo "$MASK" > "$irq" 2>/dev/null; then

        OK=$((OK+1))
    fi
done

print_ok "$OK IRQs restauradas"

# -----------------------------------------------------------------------------
# 9. Reiniciar serviços de energia
# -----------------------------------------------------------------------------

print_section "9. Reiniciando serviços"

for service in \
    power-profiles-daemon
do

    if systemctl list-unit-files "$service" >/dev/null 2>&1; then

        systemctl start "$service" 2>/dev/null

        print_ok "$service iniciado"
    fi
done

# -----------------------------------------------------------------------------
# 10. Reiniciar serviços Ubuntu
# -----------------------------------------------------------------------------

print_section "10. Reiniciando serviços Ubuntu"

for service in \
    unattended-upgrades \
    packagekit
do

    if systemctl list-unit-files "$service" >/dev/null 2>&1; then

        systemctl start "$service" 2>/dev/null
    fi
done

# -----------------------------------------------------------------------------
# STATUS FINAL
# -----------------------------------------------------------------------------

print_section "STATUS FINAL"

echo
echo "CPUs online : $(cat /sys/devices/system/cpu/online)"
echo "SMT         : $(cat /sys/devices/system/cpu/smt/control 2>/dev/null)"
echo "ASLR        : $(cat /proc/sys/kernel/randomize_va_space)"
echo "Governor    : $(cat /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null)"
echo
echo -e "${GREEN}====================================================${NC}"
echo -e "${GREEN}SISTEMA RESTAURADO${NC}"
echo -e "${GREEN}====================================================${NC}"
echo
echo "Para garantir 100% do estado original:"
echo
echo "sudo reboot"
echo
