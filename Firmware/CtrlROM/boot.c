/*	Firmware for loading files from SD card.
	Part of the ZPUTest project by Alastair M. Robinson.
	SPI and FAT code borrowed from the Minimig project.
*/


#include "stdarg.h"

#include "uart.h"
#include "spi.h"
#include "minfat.h"
#include "small_printf.h"


void _boot();
void _break();

/* Load files named in a manifest file */

int main(int argc,char **argv)
{
	int i;

	puts("Initializing SD card\n");
	if(spi_init())
	{
		puts("Hunting for partition\n");
		FindDrive();
		if(LoadFile("BIOS_M2PROM",100000))
		{
			// Spoonfeed data to OCMSX here
		}
		else
		{
			puts("Loading BIOS failed\n");
		}
	}
	puts("Returning\n");

	return(0);
}

