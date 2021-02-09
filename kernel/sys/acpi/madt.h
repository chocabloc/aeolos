#pragma once

#include "acpi.h"
#include <stdint.h>

// MADT Record Header
typedef struct {
    uint8_t type;
    uint8_t len;
} __attribute__((packed)) madt_record_hdr;

// Local APIC
typedef struct {
    madt_record_hdr hdr;

    uint8_t proc_id;
    uint8_t apic_id;
    uint32_t flags;
} __attribute__((packed)) madt_record_lapic;

// I/O APIC
typedef struct {
    madt_record_hdr hdr;

    uint8_t id;
    uint8_t reserved;
    uint32_t addr;
    uint32_t gsi_base;
} __attribute__((packed)) madt_record_ioapic;

// Interrupt Source Override
typedef struct {
    madt_record_hdr hdr;

    uint8_t bus_src;
    uint8_t irq_src;
    uint32_t gsi;
    uint16_t flags;
} __attribute__((packed)) madt_record_iso;

// Non Maskable Interrupt
typedef struct {
    madt_record_hdr hdr;

    uint8_t proc_id;
    uint16_t flags;
    uint8_t lint;
} __attribute__((packed)) madt_record_nmi;

typedef struct {
    acpi_sdt_hdr hdr;

    uint32_t lapic_addr;
    uint32_t flags;

    uint8_t records[];
} __attribute__((packed)) madt_t;

#define MADT_RECORD_TYPE_LAPIC 0
#define MADT_RECORD_TYPE_IOAPIC 1
#define MADT_RECORD_TYPE_ISO 2
#define MADT_RECORD_TYPE_NMI 4
#define MADT_RECORD_TYPE_LAPIC_AO 5

#define MADT_LAPIC_FLAG_ENABLED 1 << 0
#define MADT_LAPIC_FLAG_ONLINE_CAPABLE 1 << 1

void madt_init();
uint32_t madt_get_num_ioapic();
uint32_t madt_get_num_lapic();
madt_record_ioapic** madt_get_ioapics();
madt_record_lapic** madt_get_lapics();
uint64_t madt_get_lapic_base();
