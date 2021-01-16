#pragma once

#include <stdint.h>

#define SMP_TRAMPOLINE_BLOB_ADDR 0x1000
#define SMP_AP_BOOT_COUNTER_ADDR 0xff0

// addresses of the arguments for trampoline code
#define SMP_TRAMPOLINE_ARG_CR3 0xfd0
#define SMP_TRAMPOLINE_ARG_IDTPTR 0xfa0

// stores lapic id of cpus
typedef uint16_t cpu_t;

// smp information structure
typedef struct {
    cpu_t bsp;
    uint16_t num_ap;
    cpu_t ap[255];
} smp_info_t;

void smp_init();