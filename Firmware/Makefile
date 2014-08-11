ASDIR = ../z80asm
AS = $(ASDIR)/z80asm

PRJ = IPL

all: $(PRJ).bin $(PRJ)ROM.vhd

clean:
	rm $(PRJ).bin $(PRJ)ROM.vhd
	make -C $(ASDIR) clean

%.bin: %.asm $(AS) Makefile
	$(AS) -l$(PRJ).list -o $@ $<

%ROM.vhd: %.bin iplromgen
	cat IPL_Prologue.vhd >$@
	./iplromgen >>$@ $<
	cat IPL_Epilogue.vhd >>$@

$(AS):
	make -C $(ASDIR)

iplromgen: iplromgen.c
	gcc iplromgen.c -o iplromgen

