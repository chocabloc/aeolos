#include "serial.h"
#include "sys/cpu/cpu.h"

void serial_init()
{
    uint16_t divisor = 115200 / 9600;
    uint8_t divlow = divisor & 0xFF;
    uint8_t divhigh = divisor >> 8;
    port_outb(0x3fb, 0x80);
    port_outb(0x3f8, divlow);
    port_outb(0x3f9, divhigh);
    port_outb(0x3fb, 0x03);
    port_outb(0x3fa, 0xC7);
    port_outb(0x3fc, 0x03);
}

static int is_transmit_empty()
{
    uint8_t in;
    port_inb(0x3fd, &in);
    return in & 0x20;
}

void serial_send(char byte)
{
    while (is_transmit_empty() == 0)
        ;

    if (byte == '\n')
        port_outb(0x3f8, '\r');
    port_outb(0x3f8, byte);
}
