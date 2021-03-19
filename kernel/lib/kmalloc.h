#pragma once

#include <stdint.h>

void* kmalloc(uint64_t size);
void kmfree(void* addr);
