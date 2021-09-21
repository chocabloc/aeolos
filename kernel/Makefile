# source files
LINKSCRIPT = linker.ld
CFILES = $(shell find . -name "*.c")
ASMFILES = $(shell find . -name "*.s")
TRAMPOLINEFILE = sys/smp/trampoline.nasm
SYMBOLFILE = _symbols.gen

# object files
COBJS = $(CFILES:.c=.o)
ASMOBJS = $(ASMFILES:.s=.o)
OBJS = $(COBJS) $(ASMOBJS)
SYMBOLOBJ = _symbols.o
KERNELOBJ = kernel.elf
TRAMPOLINEBLOB = sys/smp/trampoline.bin

# toolchain options and flags
# use gcc-11 if present, else use gcc-10
# TODO: better compiler detection
CC = $(if $(shell command -v gcc-11 2> /dev/null), gcc-11, gcc-10)
LD = $(CC)
AS = $(CC)
NASM = nasm
CFLAGS = -std=gnu2x \
		 -ffreestanding \
		 -fno-pic \
		 -fno-omit-frame-pointer \
		 -flto \
		 -mcmodel=kernel \
		 -mno-red-zone \
		 -mno-80387 \
		 -mno-mmx \
		 -mno-sse \
		 -mno-sse2 \
		 -Wall \
		 -Wextra \
		 -Ofast \
		 -I . \
		 -I lib
ASFLAGS = -I . -flto
LINKFLAGS = -T$(LINKSCRIPT) \
    	    -nostdlib \
			-Wno-lto-type-mismatch \
			-z max-page-size=0x1000 \
			-flto \
			-no-pie

# rules
.PHONY: clean kernel

# compile and link symbols with the built binary
kernel: $(SYMBOLFILE) $(KERNELOBJ)
	@$(CC) $(CFLAGS) -x c -c $(SYMBOLFILE) -o $(SYMBOLOBJ)
	@echo "Linking symbols..."
	@$(LD) $(LINKFLAGS) $(OBJS) $(SYMBOLOBJ) -o $(KERNELOBJ)

# generate symbols file
$(SYMBOLFILE): $(KERNELOBJ)
	@echo "Generating symbols..."
	@./gensym $(KERNELOBJ)

# link all objects together
$(KERNELOBJ): $(TRAMPOLINEBLOB) $(OBJS) 
	@echo Linking all objects...
	@$(LD) $(LINKFLAGS) $(OBJS) -o $@
	
# build c objects
$(COBJS): %.o: %.c
	@echo Compiling $^ ...
	@$(CC) -o $@ -c $^ $(CFLAGS)

# build assembly objects
$(ASMOBJS): %.o: %.s
	@echo Compiling $^ ...
	@$(AS) -o $@ -c $^ $(ASFLAGS)

# build the trampoline blob
$(TRAMPOLINEBLOB): $(TRAMPOLINEFILE)
	@echo Compiling $^
	@$(NASM) $^ -o $@

# clean
clean: 
	@rm -f $(OBJS) $(KERNELOBJ) _symbols.gen
