#include "timer.h"
#include "apic.h"
#include "klog.h"
#include "sys/cpu/cpu.h"
#include "sys/idt.h"
#include <stdbool.h>
#include <stddef.h>

static uint64_t base_freq;
static uint8_t divisor;
static uint8_t vector;

// a dummy handler, just sends eoi
__attribute__((interrupt)) static void apic_timer_handler(void* v __attribute__((unused)))
{
    klog_warn("APIC Timer: No handler registered\n");
    apic_send_eoi();
}

void apic_timer_disable()
{
    uint32_t val = apic_read_reg(APIC_REG_TIMER_LVT);
    apic_write_reg(APIC_REG_TIMER_LVT, val | APIC_TIMER_FLAG_MASKED);
}

void apic_timer_enable()
{
    uint32_t val = apic_read_reg(APIC_REG_TIMER_LVT);
    apic_write_reg(APIC_REG_TIMER_LVT, val & ~(APIC_TIMER_FLAG_MASKED));
}

void apic_timer_set_handler(void (*h)(void))
{
    idt_set_handler(vector, h);
}

void apic_timer_set_frequency(uint64_t freq)
{
    apic_write_reg(APIC_REG_TIMER_ICR, base_freq / (freq * divisor));
}

uint8_t apic_timer_get_vector()
{
    return vector;
}

void apic_timer_set_mode(apic_timer_mode_t mode)
{
    uint32_t val = apic_read_reg(APIC_REG_TIMER_LVT);

    switch (mode) {
    case APIC_TIMER_MODE_PERIODIC:
        apic_write_reg(APIC_REG_TIMER_LVT, val | APIC_TIMER_FLAG_PERIODIC);
        break;

    default:
        apic_write_reg(APIC_REG_TIMER_LVT, val & ~(APIC_TIMER_FLAG_PERIODIC));
        break;
    }
}

// initialization for AP's
void apic_timer_init_ap()
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

    /*
     * initialize the pit as mode 0, lobyte/hibyte for calibration
     * set initial count to 0xff00, which will be decreased at about 1.2 mhz, taking 1/18.2779 s
     */
    port_outb(0x43, 0b00110000);
    port_outb(0x40, 0x00);
    port_outb(0x40, 0xff);

    // initialize the apic timer, set DCR to divide by 4
    apic_write_reg(APIC_REG_TIMER_LVT, APIC_TIMER_FLAG_MASKED | vector);
    apic_write_reg(APIC_REG_TIMER_ICR, UINT32_MAX);
    apic_write_reg(APIC_REG_TIMER_DCR, 0b0001);
    divisor = 4;

    // wait for exactly 1/18.2779 s
    while (true) {
        uint8_t lo, hi;
        port_inb(0x40, &lo);
        port_inb(0x40, &hi);
        // check for overflow
        if (hi == 0xff && lo > 0x00)
            break;
    }

    // now calculate the base frequency
    base_freq = (double)(UINT32_MAX - apic_read_reg(APIC_REG_TIMER_CCR)) * 18.277910539 * divisor;

    klog_info("APIC Timer base frequency: %d Hz. Divisor: 4.\n", base_freq);
    klog_ok("APIC Timer initialized\n");
}
