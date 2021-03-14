GENIMG = ./genimg
QEMU = qemu-system-x86_64

QEMUFLAGS =	-m 4096 \
			-no-reboot \
			-no-shutdown \
			-enable-kvm \
			-cpu qemu64,+avx \
			-smp 4 \
			-bios /usr/share/ovmf/OVMF.fd \
			-d int

KERNELDIR = kernel
KERNELFILE = kernel/kernel.elf
IMAGEFILE = os.iso

.PHONY: run $(IMAGEFILE) $(KERNELFILE) clean

$(IMAGEFILE): $(KERNELFILE)
	@echo Generating Hard Disk Image...
	@$(GENIMG)

run: $(IMAGEFILE)
	@echo Testing image in QEMU...
	@$(QEMU) -cdrom $(IMAGEFILE) $(QEMUFLAGS)
	
$(KERNELFILE): 
	@echo Building kernel...
	@$(MAKE) -C $(KERNELDIR)

clean:
	@$(MAKE) -C $(KERNELDIR) clean
