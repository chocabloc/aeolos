GENIMG = ./genimg
QEMU = qemu-system-x86_64

QEMUFLAGS =	-m 128 \
			-no-reboot \
			-no-shutdown \
			-enable-kvm \
			-cpu qemu64,+avx,+invtsc \
			-smp 4

KERNELDIR = kernel
KERNELFILE = kernel/kernel.elf
IMAGEFILE = os.iso

.PHONY: run $(IMAGEFILE) $(KERNELFILE) clean all

all: $(IMAGEFILE)

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
