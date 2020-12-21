#pragma once

#include <stdint.h>

typedef struct {
    uint64_t addr;
    char* name;
} symbol_t;

// contents automatically generated during build
extern const symbol_t _kernel_symtab[];
