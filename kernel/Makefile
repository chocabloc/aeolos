CC=clang
LD=clang
AS=clang

CFLAGS =-ffreestanding -mcmodel=kernel -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -Wall -Wextra -I . -I lib
ASFLAGS = -I .
LINKFLAGS = -Wl,-Tlinker.ld -nostdlib -Wl,--unresolved-symbols=ignore-all

CFILES = $(wildcard *.c boot/*.c dev/term/*.c dev/fb/*.c sys/*.c acpi/*.c mm/*.c lib/*.c)
ASMFILES = $(wildcard *.s boot/*.s dev/term/*.s sys/*.s)

COBJS = $(patsubst %.c, %.o, $(CFILES))
ASMOBJS = $(patsubst %.s, %.o, $(ASMFILES))

.PHONY: clean kernel symbols

kernel: kernel.elf

symbols: 
	./gensym kernel.elf
	
kernel.elf: $(COBJS) $(ASMOBJS)
	@echo Linking all objects...
	@$(LD) -o kernel.elf $^ $(LINKFLAGS)
	
$(COBJS): %.o: %.c
	@echo Compiling $< ...
	@$(CC) -o $@ -c $< $(CFLAGS)

$(ASMOBJS): %.o: %.s
	@echo Compiling $< ...
	@$(AS) -o $@ -c $< $(ASFLAGS)

clean: 
	@echo Cleaning...
	@rm -f $(COBJS) $(ASMOBJS) kernel.elf kernel.sym
