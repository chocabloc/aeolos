#pragma once

#include <stdint.h>

#define read_cr(cr, n) asm volatile("mov %%" cr ", %%rax;" \
                                    "mov %%rax, %0 "       \
                                    : "=g"(*(n))           \
                                    :                      \
                                    : "rax");

#define write_cr(cr, n) asm volatile("mov %0, %%rax;"   \
                                     "mov %%rax, %%" cr \
                                     :                  \
                                     : "g"(n)           \
                                     : "rax");

#define port_outb(port, n) asm volatile("movb %0, %%al;"    \
                                        "movw %1, %%dx;"    \
                                        "out %%al, %%dx;"   \
                                        :                   \
                                        : "g"(n), "g"(port) \
                                        : "al", "dx");

#define port_inb(port, n) asm volatile("movw %1, %%dx;" \
                                       "in %%dx, %%al;" \
                                       "movb %%al, %0"  \
                                       : "=g"(*(n))     \
                                       : "g"(port)      \
                                       : "al");

#define MSR_PAT 0x0277
#define MSR_GS_BASE 0xC0000101

void cpu_features_init();
void wrmsr(uint32_t msr, uint64_t val);
uint64_t rdmsr(uint32_t msr);
