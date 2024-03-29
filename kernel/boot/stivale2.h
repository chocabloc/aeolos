#pragma once

#include <stdint.h>

typedef struct stivale2_tag stv2_tag;

struct stivale2_tag {
    uint64_t identifier;
    stv2_tag* next;
} __attribute__((packed));

/* --- Header --------------------------------------------------------------- */
/*  Information passed from the kernel to the bootloader                      */

typedef struct stivale2_header stv2_hdr;
struct stivale2_header {
    uint64_t entry_point;
    uint64_t stack;
    uint64_t flags;
    uint64_t tags;
} __attribute__((packed));

#define STV2_HDR_TAG_FB_ID 0x3ecc1bc43d0f7971

typedef struct stivale2_header_tag_framebuffer stv2_hdr_tag_fb;

struct stivale2_header_tag_framebuffer {
    struct stivale2_tag tag;
    uint16_t fb_width;
    uint16_t fb_height;
    uint16_t fb_bpp;
} __attribute__((packed));

#define STIVALE2_HEADER_TAG_SMP_ID 0x1ab015085f3273df

struct stivale2_header_tag_smp {
    struct stivale2_tag tag;
    uint64_t flags;
} __attribute__((packed));

#define STIVALE2_HEADER_TAG_5LV_PAGING_ID 0x932f477032007e8f

/* --- Struct --------------------------------------------------------------- */
/*  Information passed from the bootloader to the kernel                      */

typedef struct stivale2_struct stivale2_struct;

struct stivale2_struct {
#define STIVALE2_BOOTLOADER_BRAND_SIZE 64
    char bootloader_brand[STIVALE2_BOOTLOADER_BRAND_SIZE];

#define STIVALE2_BOOTLOADER_VERSION_SIZE 64
    char bootloader_version[STIVALE2_BOOTLOADER_VERSION_SIZE];

    stv2_tag* tags;
} __attribute__((packed));

#define STIVALE2_STRUCT_TAG_CMDLINE_ID 0xe5e76a1b4597a781

struct stivale2_struct_tag_cmdline {
    struct stivale2_tag tag;
    uint64_t cmdline;
} __attribute__((packed));

#define STV2_STRUCT_TAG_MMAP_ID 0x2187f79e8612de07

enum {
    STIVALE2_MMAP_USABLE = 1,
    STIVALE2_MMAP_RESERVED = 2,
    STIVALE2_MMAP_ACPI_RECLAIMABLE = 3,
    STIVALE2_MMAP_ACPI_NVS = 4,
    STIVALE2_MMAP_BAD_MEMORY = 5,
    STIVALE2_MMAP_BOOTLOADER_RECLAIMABLE = 0x1000,
    STIVALE2_MMAP_KERNEL_AND_MODULES = 0x1001
};

struct stivale2_mmap_entry {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t unused;
} __attribute__((packed));

typedef struct stivale2_struct_tag_memmap stv2_struct_tag_mmap;
struct stivale2_struct_tag_memmap {
    struct stivale2_tag tag;
    uint64_t entries;
    struct stivale2_mmap_entry memmap[];
} __attribute__((packed));

#define STV2_STRUCT_TAG_FB_ID 0x506461d2950408fa

enum { STIVALE2_FBUF_MMODEL_RGB = 1 };

typedef struct stivale2_struct_tag_framebuffer stv2_struct_tag_fb;

struct stivale2_struct_tag_framebuffer {
    struct stivale2_tag tag;
    uint8_t* fb_addr;
    uint16_t fb_width;
    uint16_t fb_height;
    uint16_t fb_pitch;
    uint16_t fb_bpp;
    uint8_t memory_model;
    uint8_t red_mask_size;
    uint8_t red_mask_shift;
    uint8_t green_mask_size;
    uint8_t green_mask_shift;
    uint8_t blue_mask_size;
    uint8_t blue_mask_shift;
} __attribute__((packed));

#define STV2_STRUCT_TAG_MODULES_ID 0x4b6fe466aade04ce

struct stivale2_module {
    uint64_t begin;
    uint64_t end;

#define STIVALE2_MODULE_STRING_SIZE 128
    char string[STIVALE2_MODULE_STRING_SIZE];
} __attribute__((packed));

struct stivale2_struct_tag_modules {
    struct stivale2_tag tag;
    uint64_t module_count;
    struct stivale2_module modules[];
} __attribute__((packed));
typedef struct stivale2_struct_tag_modules stv2_struct_tag_modules;

#define STV2_STRUCT_TAG_RSDP_ID 0x9e1786930a375e78

typedef struct stivale2_struct_tag_rsdp stv2_struct_tag_rsdp;
struct stivale2_struct_tag_rsdp {
    struct stivale2_tag tag;
    uint64_t rsdp;
} __attribute__((packed));

#define STIVALE2_STRUCT_TAG_EPOCH_ID 0x566a7bed888e1407

struct stivale2_struct_tag_epoch {
    struct stivale2_tag tag;
    uint64_t epoch;
} __attribute__((packed));

#define STIVALE2_STRUCT_TAG_FIRMWARE_ID 0x359d837855e3858c

struct stivale2_struct_tag_firmware {
    struct stivale2_tag tag;
    uint64_t flags;
} __attribute__((packed));

#define STIVALE2_STRUCT_TAG_SMP_ID 0x34d1d96339647025

struct stivale2_smp_info {
    uint32_t processor_id;
    uint32_t lapic_id;
    uint64_t target_stack;
    uint64_t goto_address;
    uint64_t extra_argument;
} __attribute__((packed));

struct stivale2_struct_tag_smp {
    struct stivale2_tag tag;
    uint64_t flags;
    uint32_t bsp_lapic_id;
    uint32_t unused;
    uint64_t cpu_count;
    struct stivale2_smp_info smp_info[];
} __attribute__((packed));

void* stv2_find_struct_tag(stivale2_struct* s, uint64_t id);
