#pragma once

#include <stdint.h>

#define APIC_REG_ID 0x20
#define APIC_REG_VERSION 0x30
#define APIC_REG_SPURIOUS_INT 0xF0
#define APIC_REG_LVT_TIMER 0x320
#define APIC_REG_EOI 0xB0

void apic_init(void);