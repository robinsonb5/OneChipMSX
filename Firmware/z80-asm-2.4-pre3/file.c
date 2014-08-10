/* Routines for manipulating with Z80 machine code files */

#include <stdio.h>
#include <string.h>
#include "z80-global"


void write_header(FILE *stream,unsigned short address)
{
 unsigned char c[2];
 c[0]=address&255;
 c[1]=address>>8;
 fwrite(_Z80HEADER,1,strlen(_Z80HEADER),stream);
 fwrite(c,1,2,stream);
}


/* reads header of a file and tests if it's Z80 ASM file, reads address */
/* return value: 0=OK, 1=this is not a z80 asm file */
int read_header(FILE *stream,unsigned short *address, int *len)
{
 unsigned char tmp[9];
 unsigned char c[2];
 unsigned a,b;
 int ret=0;

 b=strlen(_Z80HEADER);
 tmp[b]=0;
 a=0;
 if ((fread(tmp,1,b,stream))!=b) ret=1;
 else if (strcmp(tmp,_Z80HEADER)) ret=1;
 else if (fread(c,1,2,stream)!=2) ret=1;
 else
 { *address=(c[1]<<8)|c[0]; a=b+2; }
 fseek(stream,0,SEEK_END);
 b=ftell(stream);
 fseek(stream,a,SEEK_SET);
 *len=b-a;
 return ret; 
}
