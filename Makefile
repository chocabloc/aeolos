GENIMG = ./genimg
QEMU = qemu-system-x86_64

QEMUFLAGS =	-m 4096 \
			-no-reboot \
			-no-shutdown \
			-enable-kvm \
			-cpu qemu64,+avx \
			-smp 4

KERNELDIR = kernel
IMAGEDIR = image

KERNELFILE = kernel.elf
IMAGEFILE = os.img

.PHONY: run $(IMAGEFILE) $(KERNELDIR)/$(KERNELFILE) clean

$(IMAGEFILE): $(IMAGEDIR)/$(KERNELFILE)
	@echo Generating Hard Disk Image...
	@$(GENIMG)

run: $(IMAGEFILE)
	@echo Testing image in QEMU...
	@$(QEMU) $(IMAGEFILE) $(QEMUFLAGS)

$(IMAGEDIR)/$(KERNELFILE) : $(KERNELDIR)/$(KERNELFILE)
	@echo Copying kernel...
	@cp $< $@
	
$(KERNELDIR)/$(KERNELFILE): 
	@echo Building kernel...
	@$(MAKE) -C $(KERNELDIR)

clean:
	@$(MAKE) -C $(KERNELDIR) clean
