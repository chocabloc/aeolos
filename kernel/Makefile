CC=clang
LD=clang
AS=clang

CFLAGS =-ffreestanding -mcmodel=kernel -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -Wall -Wextra -I . -I lib -Os
ASFLAGS = -I .
LINKFLAGS = -Wl,-Tlinker.ld -nostdlib -Os

CFILES = $(wildcard *.c boot/*.c dev/term/*.c dev/fb/*.c sys/*.c mm/*.c lib/*.c)
ASMFILES = $(wildcard *.s boot/*.s dev/term/*.s sys/*.s)

COBJS = $(patsubst %.c, %.o, $(CFILES))
ASMOBJS = $(patsubst %.s, %.o, $(ASMFILES))

.PHONY: clean kernel symbols

kernel: kernel.elf

symbols: kernel.sym

kernel.sym: kernel.elf
	@./extractsymbols kernel.elf
	
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
	@rm -f $(COBJS) $(ASMOBJS)
