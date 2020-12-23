#pragma once

#include <stdint.h>

#define APIC_REG_TIMER_LVT 0x320
#define APIC_REG_TIMER_ICR 0x380
#define APIC_REG_TIMER_CCR 0x390
#define APIC_REG_TIMER_DCR 0x3e0

#define APIC_TIMER_FLAG_PERIODIC 1 << 17
#define APIC_TIMER_FLAG_MASKED 1 << 16

typedef enum {
    APIC_TIMER_MODE_PERIODIC,
    APIC_TIMER_MODE_ONESHOT
} apic_timer_mode_t;

void apic_timer_init();
void apic_timer_disable();
void apic_timer_enable();
void apic_timer_set_handler(void* h);
void apic_timer_set_frequency(uint64_t freq);
void apic_timer_set_mode(apic_timer_mode_t mode);