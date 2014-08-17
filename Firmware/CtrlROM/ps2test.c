#include "uart.h"
#include "interrupts.h"
#include "ps2.h"


int main(int argc, char **argv)
{
	puts("Initialising PS/2 interface...\n");

	PS2Init();
	EnableInterrupts();
	while(1)
	{
		int k;
		k=HandlePS2RawCodes();
		if(k)
			putchar(k);
	}
}

