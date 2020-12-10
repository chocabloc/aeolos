#pragma once

#include <stdint.h>

typedef struct {
    uint64_t addr;
    char* name;
} symbol_t;

// contents automatically generated during build
__attribute__((weak)) extern symbol_t _kernel_symtab[];