#pragma once

#include <stdbool.h>
#include <stdint.h>

bool memcmp(const void* s1, const void* s2, uint64_t len);
void memset(void* addr, uint8_t val, uint64_t len);
void memcpy(void* src, void* target, uint64_t len);
void memcpy_fast(void* src, void* target, uint64_t len);
