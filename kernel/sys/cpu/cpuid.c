#include "cpuid.h"
#include "klog.h"

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
    uint64_t maxleaf, maxhighleaf;

    // get highest supported leaf
    cpuid(0, 0, &regs[CPUID_REG_EAX], &regs[CPUID_REG_EBX],
        &regs[CPUID_REG_ECX], &regs[CPUID_REG_EDX]);
    maxleaf = regs[CPUID_REG_EAX];

    // get highest supported extended leaf
    cpuid(0x80000000, 0, &regs[CPUID_REG_EAX], &regs[CPUID_REG_EBX],
        &regs[CPUID_REG_ECX], &regs[CPUID_REG_EDX]);
    maxhighleaf = regs[CPUID_REG_EAX];

    // is the leaf being requested supported?
    if ((maxleaf < feature.func && feature.func < 0x80000000)
        || (maxhighleaf < feature.func && feature.func >= 0x80000000)) {
        klog_warn("cpuid leaf %x not supported\n", feature.func);
        return false;
    }

    // is the feature supported?
    cpuid(feature.func, feature.param, &regs[CPUID_REG_EAX], &regs[CPUID_REG_EBX],
        &regs[CPUID_REG_ECX], &regs[CPUID_REG_EDX]);
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
