#pragma once

#include <stdint.h>

typedef volatile struct {
    int lock;
    uint64_t rflags;
} lock_t;

#define spinlock_take(s)                                        \
    {                                                           \
        asm volatile(                                           \
            "pushfq;"                                           \
            "cli;"                                              \
            "lock bts $0, %[lock];"                             \
            "jnc 2f;"                                           \
            "1:"                                                \
            "pause;"                                            \
            "bt $0, %[lock];"                                   \
            "jc 1b;"                                            \
            "lock bts $0, %[lock];"                             \
            "jc 1b;"                                            \
            "2:"                                                \
            "pop %[flags]"                                      \
            : [lock] "=m"((s)->lock), [flags] "=m"((s)->rflags) \
            :                                                   \
            : "memory", "cc");                                  \
    }

#define spinlock_release(s)                     \
    {                                           \
        asm volatile("push %[flags];"           \
                     "lock btr $0, %[lock];"    \
                     "popfq;"                   \
                     : [lock] "=m"((s)->lock)   \
                     : [flags] "m"((s)->rflags) \
                     : "memory", "cc");         \
    }
