#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint32_t func;
    uint32_t param;
    enum {
        CPUID_REG_EAX,
        CPUID_REG_EBX,
        CPUID_REG_ECX,
        CPUID_REG_EDX
    } reg;
    uint32_t mask;
} cpuid_feature_t;

static const cpuid_feature_t CPUID_FEATURE_SSE1 = { .func = 0x00000001, .reg = CPUID_REG_EDX, .mask = 1 << 25 };
static const cpuid_feature_t CPUID_FEATURE_SSE2 = { .func = 0x00000001, .reg = CPUID_REG_EDX, .mask = 1 << 26 };
static const cpuid_feature_t CPUID_FEATURE_SSE3 = { .func = 0x00000001, .reg = CPUID_REG_ECX, .mask = 1 << 0 };
static const cpuid_feature_t CPUID_FEATURE_SSSE3 = { .func = 0x00000001, .reg = CPUID_REG_ECX, .mask = 1 << 9 };
static const cpuid_feature_t CPUID_FEATURE_SSE41 = { .func = 0x00000001, .reg = CPUID_REG_ECX, .mask = 1 << 19 };
static const cpuid_feature_t CPUID_FEATURE_SSE42 = { .func = 0x00000001, .reg = CPUID_REG_ECX, .mask = 1 << 20 };
static const cpuid_feature_t CPUID_FEATURE_POPCNT = { .func = 0x00000001, .reg = CPUID_REG_ECX, .mask = 1 << 23 };
static const cpuid_feature_t CPUID_FEATURE_AVX = { .func = 0x00000001, .reg = CPUID_REG_ECX, .mask = 1 << 28 };
static const cpuid_feature_t CPUID_FEATURE_PAT = { .func = 0x00000001, .reg = CPUID_REG_EDX, .mask = 1 << 16 };
static const cpuid_feature_t CPUID_FEATURE_AVX2 = { .func = 0x00000007, .reg = CPUID_REG_EBX, .mask = 1 << 5 };

static const cpuid_feature_t CPUID_FEATURE_LZCNT = { .func = 0x80000001, .reg = CPUID_REG_ECX, .mask = 1 << 5 };
static const cpuid_feature_t CPUID_FEATURE_INVTSC = { .func = 0x80000007, .reg = CPUID_REG_EDX, .mask = 1 << 8 };

void cpuid(uint32_t func, uint32_t param, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx);
bool cpuid_check_feature(cpuid_feature_t feature);
void cpuid_get_vendor(char* vendor);
