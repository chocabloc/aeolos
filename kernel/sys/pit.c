#include "pit.h"
#include "klog.h"
#include "sys/cpu/cpu.h"
#include <stdbool.h>

void pit_wait(uint64_t ms)
{
    port_outb(0x43, 0b00110000);
    while (ms--) {
        port_outb(0x40, 0xa9);
        port_outb(0x40, 0x04);

        while (true) {
            uint8_t lo, hi;
            port_inb(0x40, &lo);
            port_inb(0x40, &hi);
            // check for overflow
            if (hi > 0x04)
                break;
        }
    }
}
