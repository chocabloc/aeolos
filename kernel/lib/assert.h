#pragma once

#include "sys/panic.h"

#define __STR(n) #n
#define _STR(n) __STR(n)
#define assert(a)                                         \
    if (!(a))                                             \
        kernel_panic("Assert (" #a ") failed at "__FILE__ \
                     " line "_STR(__LINE__) ".\n");
