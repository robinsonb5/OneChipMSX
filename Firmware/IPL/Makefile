AS = ../../z80asm/z80asm

IPL = ipl.bin

all: $(IPL)

clean:
	rm *.bin

%.bin: %.asm $(AS)
	$(AS) -o $@ $<

$(AS):
	make -C ../../z80asm

