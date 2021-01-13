#include "madt.h"
#include "klog.h"
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

    uint64_t size = madt->hdr.length - sizeof(madt_t);
    for (uint64_t i = 0; i < size;) {
        madt_record_hdr* rec = (madt_record_hdr*)(madt->records + i);
        switch (rec->type) {

        case MADT_RECORD_TYPE_LAPIC: {
            // we support only 256 lapic's
            if (num_lapic > 256)
                break;

            madt_record_lapic* lapic = (madt_record_lapic*)rec;
            lapics[num_lapic++] = lapic;
        } break;

        case MADT_RECORD_TYPE_IOAPIC: {
            // we support only 2 ioapic's
            if (num_ioapic > 2)
                break;

            madt_record_ioapic* ioapic = (madt_record_ioapic*)rec;
            io_apics[num_ioapic++] = ioapic;
        } break;

            // TODO: Handle MADT_RECORD_TYPE_ISO and MADT_RECORD_TYPE_NMI
        }
        i += rec->len;
    }

    klog_ok("MADT initialized\n");
}