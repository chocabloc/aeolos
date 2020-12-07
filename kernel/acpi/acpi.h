#pragma once

#include "boot/stivale2.h"

typedef struct {
    char sign[8];
    uint8_t chksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_addr;

    // intoduced in version 2.0
    uint32_t length;
    uint64_t xsdt_addr;
    uint8_t chksum_ext;
    uint8_t reserved[3];
} __attribute__((packed)) rsdp_t;

typedef struct {
    // the header
    char sign[4];
    uint32_t length;
    uint8_t rev;
    uint8_t chksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_rev;
    uint32_t creator_id;
    uint32_t creator_rev;

    // the data
    uint8_t data[];
} __attribute__((packed)) acpi_sdt;

#define SDT_SIGN_MADT "APIC"
#define SDT_SIGN_BGRT "BGRT"

void acpi_init(stv2_struct_tag_rsdp*);
acpi_sdt* acpi_get_sdt(const char* sign);