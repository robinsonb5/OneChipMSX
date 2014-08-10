#ifndef __FILE_H
#define __FILE_H

extern void write_header(FILE *stream,unsigned short address);
extern int read_header(FILE *stream, unsigned short *address,int *len);

#endif
