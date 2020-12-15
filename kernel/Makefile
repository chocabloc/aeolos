KERNELFILE = kernel.elf
LINKSCRIPT = linker.ld

CFILES = $(wildcard *.c boot/*.c dev/term/*.c dev/fb/*.c sys/*.c acpi/*.c mm/*.c lib/*.c)
ASMFILES = $(wildcard *.s boot/*.s dev/term/*.s sys/*.s)

COBJS = $(patsubst %.c, %.o, $(CFILES))
ASMOBJS = $(patsubst %.s, %.o, $(ASMFILES))

SYMBOLFILE = symbols.c
SYMBOLOBJ = symbols.o

CC = clang
LD = clang
AS = clang
GENSYM = ./gensym

CFLAGS = -ffreestanding \
		 -mcmodel=kernel \
		 -mno-red-zone \
		 -mno-mmx \
		 -mno-sse \
		 -mno-sse2 \
		 -Wall \
		 -Wextra \
		 -Ofast \
		 -fno-omit-frame-pointer \
		 -I . \
		 -I lib

ASFLAGS = -I .
LINKFLAGS = -Wl,-T$(LINKSCRIPT) -nostdlib

.PHONY: clean kernel

kernel: $(KERNELFILE)
	
$(KERNELFILE): $(COBJS) $(ASMOBJS)
	@echo Linking all objects...
	@$(LD) -o $@ $^ $(LINKFLAGS) && \
	echo "Generating symbols..." && \
	./$(GENSYM) $@ && \
	$(CC) -o $(SYMBOLOBJ) -c $(SYMBOLFILE) $(CFLAGS) && \
	echo "Linking symbols..." && \
	$(LD) -o $(KERNELFILE) $^ $(LINKFLAGS)
	
$(COBJS): %.o: %.c
	@echo Compiling $^ ...
	@$(CC) -o $@ -c $^ $(CFLAGS)

$(ASMOBJS): %.o: %.s
	@echo Compiling $^ ...
	@$(AS) -o $@ -c $^ $(ASFLAGS)

clean: 
	@echo Cleaning...
	@rm -f $(COBJS) $(ASMOBJS) $(KERNELFILE)
