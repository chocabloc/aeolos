#pragma once

typedef volatile int spinlock_t;

#define spinlock_take(s)                             \
    while (!__sync_bool_compare_and_swap(s, 0, 1)) { \
        asm volatile("pause");                       \
    }

#define spinlock_release(s) *s = 0
