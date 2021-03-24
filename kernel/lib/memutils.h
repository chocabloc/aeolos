#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool memcmp(const void* s1, const void* s2, uint64_t len);
void memset(void* addr, uint8_t val, uint64_t len);
void memcpy(const void* src, void* target, uint64_t len);
size_t strlen(const char* str);
int strcmp(const char* a, const char* b);
int strncmp(const char* a, const char* b, size_t len);
int strcpy(const char* src, char* dest);
