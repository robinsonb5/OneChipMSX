/*	Firmware for loading files from SD card.
	Part of the ZPUTest project by Alastair M. Robinson.
	SPI and FAT code borrowed from the Minimig project.
*/


#include "stdarg.h"

#include "uart.h"
#include "spi.h"
#include "minfat.h"
#include "small_printf.h"
#include "host.h"

fileTYPE file; // Use the file defined in minfat.h to avoid another instance taking up ROM space


int main(int argc,char **argv)
{
	int i;
	HW_HOST(HW_HOST_CTRL)=HW_HOST_CTRLF_RESET;	// Put OCMS into Reset
	HW_HOST(HW_HOST_SW)=0x39; // Default DIP switch settings
	HW_HOST(HW_HOST_CTRL)=HW_HOST_CTRLF_SDCARD;	// Release reset but steal SD card

	puts("Initializing SD card\n");
	if(spi_init())
	{
		puts("Hunting for partition\n");
		FindDrive();

		if(FileOpen(&file,"BIOS_M2PROM"))
		{
			puts("Opened file, loading...\n");
			int filesize=(file.size+511)/512;
			int c=0;


			while(c<filesize)
			{
				if(FileRead(&file,sector_buffer))
				{
					int i;
					int *p=(int *)&sector_buffer;
					if(!c)
						HW_HOST(HW_HOST_BOOTDATA)=0; // Write one dummy byte since firmware discards the first byte
					for(i=0;i<512;i+=4)
					{
						int t=*p++;
						int t1=t*255;
						int t2=(t>>8)&255;
						int t3=(t>>16)&255;
						int t4=(t>>24)&255;
						HW_HOST(HW_HOST_BOOTDATA)=t4;
						HW_HOST(HW_HOST_BOOTDATA)=t3;
						HW_HOST(HW_HOST_BOOTDATA)=t2;
						HW_HOST(HW_HOST_BOOTDATA)=t1;
						putchar('+');
					}
				}
				else
					puts("Read block failed\n");
				FileNextSector(&file);
				++c;
				putchar('.');
			}
		}
		else
		{
			puts("Loading BIOS failed\n");
		}
	}
	HW_HOST(HW_HOST_CTRL)=0;	// Release SD card

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

	puts("Returning\n");

	return(0);
}

