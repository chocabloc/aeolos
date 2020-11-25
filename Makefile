IMAGEFILE = os.img

KERNELDIR = kernel
IMAGEDIR = image

KERNEL = $(KERNELDIR)/kernel.elf

.PHONY: run $(KERNEL) clean
	
$(IMAGEFILE): $(KERNEL)
	cp $(KERNEL) $(IMAGEDIR)/kernel.elf && ./genimg
	
$(KERNEL): 
	$(MAKE) -C $(KERNELDIR)
	
run: $(IMAGEFILE)
	qemu-system-x86_64 $(IMAGEFILE) -bios /usr/share/ovmf/OVMF.fd -m 1024 -enable-kvm
	
clean:
	$(MAKE) -C $(KERNELDIR) clean
