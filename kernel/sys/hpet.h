#pragma once

#include "acpi/acpi.h"
#include <stdint.h>

typedef struct {
    // the header
    acpi_sdt_hdr hdr;

    // event timer block id
    uint8_t hardware_rev_id;
    uint8_t comparator_count : 5;
    uint8_t counter_size : 1;
    uint8_t reserved : 1;
    uint8_t legacy_replace : 1;
    uint16_t pci_vendor_id;

    // base address
    acpi_gas_t base_addr;

    uint8_t hpet_number;
    uint16_t minimum_tick;
    uint8_t page_protection;
} __attribute__((packed)) hpet_sdt_t;

#define HPET_REG_GEN_CAP_ID 0x00
#define HPET_REG_GEN_CONF 0x10
#define HPET_REG_GEN_INT_STATUS 0x20
#define HPET_REG_MAIN_CNT_VAL 0xf0

#define HPET_FLAG_ENABLE_CNF (1 << 0)

void hpet_init();
uint64_t hpet_get_nanos();
void hpet_nanosleep(uint64_t nanos);
