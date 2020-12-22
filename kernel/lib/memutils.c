#include "memutils.h"

void memcpy(void* src, void* target, uint64_t len)
{
    uint8_t* s = (uint8_t*)src;
    uint8_t* t = (uint8_t*)target;

    for (uint64_t i = 0; i < len; i++)
        t[i] = s[i];
}

// fast sse memcpy
void memcpy_fast(void* src, void* target, uint64_t len)
{
    asm volatile("mov %[in], %%rax;"
                 "mov %[out], %%rbx;"
                 "mov %[len], %%rcx;"
                 "mov %%rax, %%rdx;"
                 "add %[len], %%rdx;"
                 "0:"
                 "movdqa (%%rax), %%xmm0;"
                 "movdqa %%xmm0, (%%rbx);"
                 "add $16, %%rax;"
                 "add $16, %%rbx;"
                 "cmp %%rax, %%rdx;"
                 "jge 0b"
                 : [ out ] "=g"(target)
                 : [ in ] "g"(src), [ len ] "g"(len)
                 : "xmm0", "rax", "rbx", "rcx", "rdx");
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