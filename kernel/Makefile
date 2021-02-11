# source files
LINKSCRIPT = linker.ld
CFILES = $(shell find . -name "*.c")
ASMFILES = $(shell find . -name "*.s")
TRAMPOLINEFILE = sys/smp/trampoline.nasm

# object files
COBJS = $(CFILES:.c=.o)
ASMOBJS = $(ASMFILES:.s=.o)
OBJS = $(COBJS) $(ASMOBJS)
SYMBOLS = _symbols.o
KERNELOBJ = kernel.elf
TRAMPOLINEBLOB = sys/smp/trampoline.bin

# toolchain flags and settings
CC = clang-10
LD = clang-10
AS = clang-10
NASM = nasm
CFLAGS = -ffreestanding \
		 -mcmodel=kernel \
		 -mno-red-zone \
		 -mno-mmx \
		 -mno-sse \
		 -mno-sse2 \
		 -Wall \
		 -Wextra \
		 -Ofast \
		 -flto \
		 -fno-omit-frame-pointer \
		 -I . \
		 -I lib
ASFLAGS = -I . -flto
LINKFLAGS = -Wl,-T$(LINKSCRIPT) -nostdlib -z max-page-size=0x1000 -flto

# rules
.PHONY: clean kernel

kernel: $(KERNELOBJ)

# link, then generate symbols, and then link again	
$(KERNELOBJ): $(TRAMPOLINEBLOB) $(OBJS) 
	@echo Linking all objects...
	@$(LD) $(LINKFLAGS) $(OBJS) -o $@ 

	@echo "Generating symbols..."
	@./gensym $@
	@$(CC) $(CFLAGS) -x c -c _symbols.gen -o _symbols.o

	@echo "Linking symbols..."
	@$(LD) $(LINKFLAGS) $(OBJS) _symbols.o -o $@ 
	
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
	@echo Cleaning...
	@rm -f $(OBJS) $(KERNELOBJ) _symbols.gen
