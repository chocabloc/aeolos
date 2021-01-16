#pragma once

#include <stdbool.h>
#include <stdint.h>

#define SMP_TRAMPOLINE_BLOB_ADDR 0x1000
#define SMP_AP_BOOT_COUNTER_ADDR 0xff0

// addresses of the arguments for trampoline code
#define SMP_TRAMPOLINE_ARG_CR3 0xfd0
#define SMP_TRAMPOLINE_ARG_IDTPTR 0xfa0

typedef struct {
    uint16_t lapic_id;
    bool is_bsp;
} cpu_t;

// smp information structure
typedef struct {
    uint16_t num_cpus;
    cpu_t cpus[255];
} smp_info_t;

void smp_init();