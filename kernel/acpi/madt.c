#include "madt.h"
#include "kconio.h"
#include "sys/panic.h"

static madt_t* madt;

static uint64_t num_lapic;
static madt_record_lapic* lapics[256];

static uint64_t num_ioapic;
static madt_record_ioapic* io_apics[4];

uint32_t madt_get_num_ioapic() { return num_ioapic; }
uint32_t madt_get_num_lapic() { return num_lapic; }

madt_record_ioapic** madt_get_ioapics() { return io_apics; }
madt_record_lapic** madt_get_lapics() { return lapics; }

uint64_t madt_get_lapic_base() { return madt->lapic_addr; }

void madt_init()
{
    madt = (madt_t*)acpi_get_sdt(SDT_SIGN_MADT);

    if (!madt)
        kernel_panic("MADT not found\n");

    kdbg_info("LAPIC address: %x, Flags: %x\n", madt->lapic_addr, madt->flags);

    uint64_t size = madt->hdr.length - sizeof(madt_t);
    for (uint64_t i = 0; i < size;) {
        madt_record_hdr* rec = (madt_record_hdr*)(madt->records + i);
        switch (rec->type) {
        case MADT_RECORD_TYPE_LAPIC: {
            madt_record_lapic* lapic = (madt_record_lapic*)rec;
            kdbg_info("Local APIC found. Processor ID: %d\n", lapic->proc_id, lapic->apic_id);
            lapics[num_lapic++] = lapic;
        } break;

        case MADT_RECORD_TYPE_IOAPIC: {
            madt_record_ioapic* ioapic = (madt_record_ioapic*)rec;
            kdbg_info("I/O APIC found. Address: %x\n", ioapic->addr);
            io_apics[num_ioapic++] = ioapic;
            if (num_ioapic >= 4)
                kernel_panic("More than 4 I/O APIC's found!");
        } break;

        case MADT_RECORD_TYPE_ISO: {
            madt_record_iso* iso = (madt_record_iso*)rec;
            kdbg_info("Interrupt Source Override found. IRQ src: %d\n", iso->irq_src);
        } break;

        case MADT_RECORD_TYPE_NMI: {
            madt_record_nmi* nmi = (madt_record_nmi*)rec;
            kdbg_info("NMI found: LINT%d\n", nmi->lint);
        } break;

        case MADT_RECORD_TYPE_LAPIC_AO: {
            madt_record_lapic_ao* ao = (madt_record_lapic_ao*)rec;
            kdbg_info("LAPIC AO found. Processor ID: %d. New Addr: %x\n", ao->proc_id, ao->addr);
        } break;

        default:
            break;
        }
        i += rec->len;
    }
}