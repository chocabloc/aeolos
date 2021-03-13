.global smp_trampoline_blob_start
.global smp_trampoline_blob_end

smp_trampoline_blob_start:
    .incbin "sys/smp/trampoline.bin"
smp_trampoline_blob_end:
