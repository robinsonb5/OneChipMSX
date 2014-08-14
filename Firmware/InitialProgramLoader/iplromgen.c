// zpuromgen.c
//
// Program to turn a binary file into a VHDL lookup table.
//   by Adam Pierce
//   29-Feb-2008
//
// This software is free to use by anyone for any purpose.
//

#include <unistd.h>
#include <stdio.h>

typedef unsigned char BYTE;

main(int argc, char **argv)
{
       BYTE    opcode[4];
       int     fd;
       int     addr = 0;
       ssize_t s;

// Check the user has given us an input file.
       if(argc < 2)
       {
               printf("Usage: %s <binary_file>\n\n", argv[0]);
               return 1;
       }

// Open the input file.
       fd = open(argv[1], 0);
       if(fd == -1)
       {
               perror("File Open");
               return 2;
       }

       while(1)
       {

       // Read 8 bits.
               s = read(fd, opcode, 1);
               if(s == -1)
               {
                       perror("File read");
                       return 3;
               }

               if(s == 0)
                       break; // End of file.

				if(addr==0)
					printf("\t\t");
				else if((addr&7)==0)
					printf(",\n\t\t");
				else
					printf(",");

       // Output to STDOUT.
               printf("X\"%02x\"",
                      opcode[0]);
				++addr;
       }
		printf(",\n\t\tothers=>X\"00\"");

       close(fd);
       return 0;
}

