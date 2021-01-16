#pragma once

#define SMP_TRAMPOLINE_BLOB_ADDR 0x1000
#define SMP_AP_BOOT_COUNTER_ADDR 0xff0

// addresses of the arguments for trampoline code
#define SMP_TRAMPOLINE_ARG_CR3 0xfd0
#define SMP_TRAMPOLINE_ARG_IDTPTR 0xfa0

void smp_init();