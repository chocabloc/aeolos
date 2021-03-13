#include "cpuid.h"

void cpuid(uint32_t func, uint32_t param, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx)
{
    asm volatile("mov %[func], %%eax;"
                 "mov %[param], %%ecx;"
                 "cpuid;"
                 "mov %%eax, %[ieax];"
                 "mov %%ebx, %[iebx];"
                 "mov %%ecx, %[iecx];"
                 "mov %%edx, %[iedx];"
                 : [ieax] "=g"(*eax), [iebx] "=g"(*ebx), [iecx] "=g"(*ecx), [iedx] "=g"(*edx)
                 : [func] "g"(func), [param] "g"(param)
                 : "%eax", "%ebx", "%ecx", "%edx", "memory");
}

bool cpuid_check_feature(cpuid_feature_t feature)
{
    uint32_t regs[4];
    cpuid(feature.func, feature.param, &regs[cpuid_eax], &regs[cpuid_ebx], &regs[cpuid_ecx], &regs[cpuid_edx]);

    if (regs[feature.reg] & feature.mask)
        return true;

    return false;
}

void cpuid_get_vendor(char* vendor)
{
    uint32_t _eax;
    cpuid(0, 0, &_eax, (uint32_t*)vendor, (uint32_t*)(vendor + 8), (uint32_t*)(vendor + 4));
    vendor[12] = '\0';
}
