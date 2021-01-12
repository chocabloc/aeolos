#include "cpu.h"
#include "cpuid.h"
#include "klog.h"

uint64_t rdmsr(uint32_t msr)
{
    uint32_t msrlow;
    uint32_t msrhigh;

    asm volatile("mov %[msr], %%ecx;"
                 "rdmsr;"
                 "mov %%eax, %[msrlow];"
                 "mov %%edx, %[msrhigh];"
                 : [ msrlow ] "=g"(msrlow), [ msrhigh ] "=g"(msrhigh)
                 : [ msr ] "g"(msr)
                 : "eax", "ecx", "edx");

    uint64_t msrval = ((uint64_t)msrhigh << 32) | msrlow;
    return msrval;
}

void wrmsr(uint32_t msr, uint64_t val)
{
    uint32_t msrlow = val & UINT32_MAX;
    uint32_t msrhigh = (val >> 32) & UINT32_MAX;

    asm volatile("mov %[msr], %%ecx;"
                 "mov %[msrlow], %%eax;"
                 "mov %[msrhigh], %%edx;"
                 "wrmsr;"
                 :
                 : [ msr ] "g"(msr), [ msrlow ] "g"(msrlow), [ msrhigh ] "g"(msrhigh)
                 : "eax", "ecx", "edx");
}

// enable cpu features like sse2
void cpu_features_init()
{
    klog_info("CPU Information: \n");

    char vendor[13];
    cpuid_get_vendor(vendor);
    klog_printf(" \tVendor: %s\n", vendor);
    klog_printf(" \tSSE1: %b\n", cpuid_check_feature(CPUID_FEATURE_SSE1));
    klog_printf(" \tSSE2: %b\n", cpuid_check_feature(CPUID_FEATURE_SSE2));
    klog_printf(" \tSSE3: %b\n", cpuid_check_feature(CPUID_FEATURE_SSE3));
    klog_printf(" \tAVX:  %b\n", cpuid_check_feature(CPUID_FEATURE_AVX));
    klog_printf(" \tAVX2: %b\n", cpuid_check_feature(CPUID_FEATURE_AVX2));

    // clear the CR0.EM bit and set the CR0.MP bit
    uint64_t vcr0;
    read_cr("cr0", &vcr0);
    vcr0 &= ~(1 << 2);
    vcr0 |= 1 << 1;
    write_cr("cr0", vcr0);

    // set the CR4.OSFXSR and CR4.OSXMMEXCPT bit
    uint64_t vcr4;
    read_cr("cr4", &vcr4);
    vcr4 |= 1 << 9;
    vcr4 |= 1 << 10;
    write_cr("cr4", vcr4);

    // we do not have any support for saving state yet :(

    klog_ok("CPU features initialized\n");
}