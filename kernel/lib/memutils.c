#include "memutils.h"

void memcpy(void* src, void* target, uint64_t len)
{
    asm volatile("mov %[len], %%rcx;"
                 "mov %[src], %%rsi;"
                 "mov %[tgt], %%rdi;"
                 "rep movsb;"
                 :
                 : [len] "g"(len), [src] "g"(src), [tgt] "g"(target)
                 : "memory");
}

void memset(void* addr, uint8_t val, uint64_t len)
{
    uint8_t* a = (uint8_t*)addr;
    for (uint64_t i = 0; i < len; i++)
        a[i] = val;
}

bool memcmp(const void* s1, const void* s2, uint64_t len)
{
    for (uint64_t i = 0; i < len; i++) {
        uint8_t a = ((uint8_t*)s1)[i];
        uint8_t b = ((uint8_t*)s2)[i];

        if (a != b)
            return false;
    }
    return true;
}