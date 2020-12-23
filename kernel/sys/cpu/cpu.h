#pragma once

#include <stdint.h>

#define read_cr(cr, n) asm("mov %%" cr ", %%rax;" \
                           "mov %%rax, %0 "       \
                           : "=g"(*n)             \
                           :                      \
                           : "rax");

#define write_cr(cr, n) asm("mov %0, %%rax;"   \
                            "mov %%rax, %%" cr \
                            :                  \
                            : "g"(n)           \
                            : "rax");

#define MSR_PAT 0x0277

void cpu_features_init();
void wrmsr(uint32_t msr, uint64_t val);
uint64_t rdmsr(uint32_t msr);