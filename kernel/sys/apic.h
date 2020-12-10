#pragma once

#include <stdint.h>

#define APIC_REG_ID 0x20
#define APIC_REG_VERSION 0x30

void apic_init();