GENIMG = ./genimg
QEMU = qemu-system-x86_64
SAF-MAKE = ./thirdparty/saf/saf-make

QEMUFLAGS =	-m 128 \
			-no-reboot \
			-no-shutdown \
			-smp 4 \
			-cpu host \
			-M accel=kvm:tcg

KERNELDIR = kernel
KERNELFILE = kernel/kernel.elf
INITRDDIR = initrd
INITRDFILE = image/boot/initrd.saf.gz
IMAGEFILE = os.iso

.PHONY: run $(IMAGEFILE) $(KERNELFILE) clean all

all: $(IMAGEFILE)

run: $(IMAGEFILE)
	@echo Testing image in QEMU...
	@$(QEMU) -cdrom $(IMAGEFILE) $(QEMUFLAGS)

$(IMAGEFILE): $(KERNELFILE) $(INITRDFILE)
	@echo Generating Hard Disk Image...
	@$(GENIMG)

$(INITRDFILE): $(INITRDDIR) $(SAF-MAKE)
	@echo Generating initrd...
	@$(SAF-MAKE) $(INITRDDIR) image/boot/initrd.saf -q
	@gzip -f image/boot/initrd.saf 

$(SAF-MAKE):
	@echo Building tools...
	@$(MAKE) -C thirdparty/saf all
	
$(KERNELFILE): 
	@echo Building kernel...
	@$(MAKE) -C $(KERNELDIR)

clean:
	@echo Cleaning...
	@$(MAKE) -C $(KERNELDIR) clean
	@$(MAKE) -C thirdparty/saf clean
	@rm -f $(INITRDFILE)
