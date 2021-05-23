#include "timer.h"
#include "apic.h"
#include "klog.h"
#include "lib/time.h"
#include "sys/hpet.h"
#include "sys/idt.h"

static uint64_t base_freq;
static uint8_t divisor;
static uint8_t vector;

// a dummy handler, just sends eoi
[[gnu::interrupt]] static void apic_timer_handler(void* v)
{
    (void)v;
    klog_warn("APIC Timer: No handler registered\n");
    apic_send_eoi();
}

void apic_timer_stop()
{
    uint32_t val = apic_read_reg(APIC_REG_TIMER_LVT);
    apic_write_reg(APIC_REG_TIMER_LVT, val | APIC_TIMER_FLAG_MASKED);
}

void apic_timer_start()
{
    uint32_t val = apic_read_reg(APIC_REG_TIMER_LVT);
    apic_write_reg(APIC_REG_TIMER_LVT, val & ~(APIC_TIMER_FLAG_MASKED));
}

void apic_timer_set_handler(void (*h)(void*))
{
    idt_set_handler(vector, h);
}

void apic_timer_set_frequency(uint64_t freq)
{
    apic_write_reg(APIC_REG_TIMER_ICR, base_freq / (freq * divisor));
}

void apic_timer_set_period(timeval_t tv)
{
    uint64_t freq = 1000000000 / tv;
    apic_timer_set_frequency(freq);
}

uint8_t apic_timer_get_vector()
{
    return vector;
}

void apic_timer_set_mode(apic_timer_mode_t mode)
{
    uint32_t val = apic_read_reg(APIC_REG_TIMER_LVT);

    if(mode == APIC_TIMER_MODE_PERIODIC)
        apic_write_reg(APIC_REG_TIMER_LVT, val | APIC_TIMER_FLAG_PERIODIC);
    else
        apic_write_reg(APIC_REG_TIMER_LVT, val & ~(APIC_TIMER_FLAG_PERIODIC));
}

void apic_timer_enable()
{
    apic_write_reg(APIC_REG_TIMER_LVT, APIC_TIMER_FLAG_MASKED | vector);
    apic_write_reg(APIC_REG_TIMER_ICR, UINT32_MAX);
    apic_write_reg(APIC_REG_TIMER_DCR, 0b0001);
}

void apic_timer_init()
{
    // allocate a vector for the timer
    vector = idt_get_vector();
    idt_set_handler(vector, &apic_timer_handler);

    // unmask the apic timer interrupt and set divisor to 4
    apic_write_reg(APIC_REG_TIMER_LVT, APIC_TIMER_FLAG_MASKED | vector);
    apic_write_reg(APIC_REG_TIMER_DCR, 0b0001);
    divisor = 4;

    // calibrate the timer
    apic_write_reg(APIC_REG_TIMER_ICR, UINT32_MAX);
    hpet_nanosleep(MILLIS_TO_NANOS(500));
    base_freq = ((UINT32_MAX - apic_read_reg(APIC_REG_TIMER_CCR)) * 2) * divisor;

    klog_info("APIC Timer base frequency: %d Hz. Divisor: 4.\n", base_freq);
    klog_ok("done\n");
}
