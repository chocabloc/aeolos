#pragma once

typedef volatile struct {
    int lock;
    uint64_t rflags;
} spinlock_t;

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
            : [lock] "=g"((s)->lock), [flags] "=g"((s)->rflags) \
            :                                                   \
            : "memory", "cc");                                  \
    }

#define spinlock_release(s)                     \
    {                                           \
        asm volatile("push %[flags];"           \
                     "lock btr $0, %[lock];"    \
                     "popfq;"                   \
                     : [lock] "=g"((s)->lock)   \
                     : [flags] "g"((s)->rflags) \
                     : "memory", "cc");         \
    }

#define spinlock_take_noirq(s)                                    \
    {                                                             \
        while (!__sync_bool_compare_and_swap(&((s)->lock), 0, 1)) \
            asm volatile("pause");                                \
    }

#define spinlock_release_noirq(s) \
    {                             \
        (s)->lock = 0;            \
    }
