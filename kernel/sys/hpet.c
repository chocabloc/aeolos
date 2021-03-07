#include "hpet.h"
#include "acpi/acpi.h"
#include "klog.h"
#include "mm/mm.h"
#include "panic.h"

static void* hpet_regs;
static uint64_t hpet_period;

static uint64_t hpet_read_reg(uint16_t offset)
{
    return *((volatile uint64_t*)(hpet_regs + offset));
}

static void hpet_write_reg(uint16_t offset, uint64_t val)
{
    *((volatile uint64_t*)(hpet_regs + offset)) = val;
}

// get current time in nanoseconds
uint64_t hpet_get_nanos()
{
    uint64_t tf = hpet_read_reg(HPET_REG_MAIN_CNT_VAL) * hpet_period;
    return tf;
}

void hpet_nanosleep(uint64_t nanos)
{
    uint64_t tgt = hpet_get_nanos() + nanos;
    while (hpet_get_nanos() < tgt)
        ;
}

void hpet_init()
{
    hpet_sdt_t* hpet_sdt = (hpet_sdt_t*)acpi_get_sdt(SDT_SIGN_HPET);
    if (!hpet_sdt)
        kernel_panic("HPET not found\n");

    // map the hpet registers
    uint64_t hpet_phys = hpet_sdt->base_addr.address;
    vmm_map(PHYS_TO_VIRT(hpet_phys), hpet_phys, 1, FLAG_PRESENT | FLAG_READWRITE | FLAG_CACHE_DISABLE);
    hpet_regs = (void*)PHYS_TO_VIRT(hpet_phys);

    // get time period in nanoseconds
    hpet_period = (hpet_read_reg(HPET_REG_GEN_CAP_ID) >> 32) / 1000000;

    // enable the counter
    hpet_write_reg(HPET_REG_GEN_CONF, hpet_read_reg(HPET_REG_GEN_CONF) | HPET_FLAG_ENABLE_CNF);
    klog_ok("HPET initialized\n");
}