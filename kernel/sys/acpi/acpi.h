#pragma once

#include "boot/stivale2.h"

typedef struct {
    char sign[8];
    uint8_t chksum;
    char OEMID[6];
    uint8_t revision;
    uint32_t rsdt_addr;

    // intoduced in version 2.0
    uint32_t length;
    uint64_t xsdt_addr;
    uint8_t chksum_ext;
    uint8_t reserved[3];
} __attribute__((packed)) rsdp_t;

void acpi_init(stv2_struct_tag_rsdp*);