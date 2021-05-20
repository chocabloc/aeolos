#include "memutils.h"
#include <stddef.h>

void memcpy(const void* src, void* target, uint64_t len)
{
    asm volatile("mov %[len], %%rcx;"
                 "mov %[src], %%rsi;"
                 "mov %[tgt], %%rdi;"
                 "rep movsb;"
                 :
                 : [len] "g"(len), [src] "g"(src), [tgt] "g"(target)
                 : "memory", "rcx", "rsi", "rdi");
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

size_t strlen(const char* str)
{
    size_t len;
    for (len = 0; str[len] != '\0'; len++)
        ;
    return len;
}

int strcmp(const char* a, const char* b)
{
    for (size_t i = 0;; i++) {
        if (a[i] != b[i] || a[i] == '\0' || b[i] == '\0')
            return a[i] - b[i];
    }
}

int strncmp(const char* a, const char* b, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        if (a[i] != b[i] || a[i] == '\0' || b[i] == '\0')
            return a[i] - b[i];
    }
    return 0;
}

int strcpy(const char* src, char* dest)
{
    size_t i;
    for (i = 0;; i++) {
        dest[i] = src[i];
        if (src[i] == '\0')
            break;
    }
    return i;
}
