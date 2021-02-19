#pragma once

#include <stdint.h>

typedef struct {
    uint32_t s;
    uint32_t ms;
    uint32_t us;
    uint32_t ns;
} timeval_t;