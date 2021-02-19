#pragma once

typedef volatile struct {
    int lock;
    uint64_t rflags;
} spinlock_t;

#define spinlock_take(s)                                          \
    {                                                             \
        asm volatile("pushfq;"                                    \
                     "pop %0;"                                    \
                     "cli;"                                       \
                     : "=g"((s)->rflags)                          \
                     :                                            \
                     : "memory", "cc");                           \
        while (!__sync_bool_compare_and_swap(&((s)->lock), 0, 1)) \
            asm volatile("pause");                                \
    }

#define spinlock_release(s)             \
    {                                   \
        (s)->lock = 0;                  \
        asm volatile("push %0;"         \
                     "popfq;"           \
                     :                  \
                     : "g"((s)->rflags) \
                     : "memory", "cc"); \
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
