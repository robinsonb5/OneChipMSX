SYSTEM=UNIX

DEFINES=-DFURTHER_HARDWARE -DPORT_BUFFER

ASM=z80-asm

CC=	gcc
CFLAGS=-O2 -Wall -D$(SYSTEM) $(DEFINES) -W -Wstrict-prototypes \
       -Wno-parentheses -fomit-frame-pointer -falign-functions=0
LDFLAGS=

all:
	make $(ASM)

clean:
	rm *.o
	rm $(ASM)

MAIN_SRC = z80-asm.c asm.c compile.c instr.c hash.c dummy.c expression.c file.c regs.c
MAIN_OBJ = $(patsubst %.c,%.o,$(MAIN_SRC))

$(ASM): $(MAIN_OBJ)
	gcc -o $@ $+ $(LIBS)

%.o: %.c Makefile
	$(CC) $(CFLAGS)  -o $@ -c $<

