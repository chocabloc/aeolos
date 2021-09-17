#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef volatile struct {
    int lock;
    uint64_t rflags;
} lock_t;

typedef volatile struct {
    int num_readers;
    lock_t nrlock;
    lock_t wlock;
} rwlock_t;

#define lock_wait(s)                                            \
    {                                                           \
        asm volatile(                                           \
            "pushfq;"                                           \
            "cli;"                                              \
            "lock btsl $0, %[lock];"                             \
            "jnc 2f;"                                           \
            "1:"                                                \
            "pause;"                                            \
            "btl $0, %[lock];"                                   \
            "jc 1b;"                                            \
            "lock btsl $0, %[lock];"                             \
            "jc 1b;"                                            \
            "2:"                                                \
            "pop %[flags]"                                      \
            : [lock] "=m"((s)->lock), [flags] "=m"((s)->rflags) \
            :                                                   \
            : "memory", "cc");                                  \
    }

#define lock_release(s)                         \
    {                                           \
        asm volatile("push %[flags];"           \
                     "lock btrl $0, %[lock];"    \
                     "popfq;"                   \
                     : [lock] "=m"((s)->lock)   \
                     : [flags] "m"((s)->rflags) \
                     : "memory", "cc");         \
    }

#define lock_try(s) __sync_bool_compare_and_swap(&((s)->lock), 0, 1)

bool rwlock_try_read(rwlock_t* l);
void rwlock_end_read(rwlock_t* l);
bool rwlock_try_write(rwlock_t* l);
void rwlock_end_write(rwlock_t* l);
