#ifndef __Z80ASM_H
#define __Z80ASM_H

extern void error(int l,char *line,char *txt);
extern unsigned char write_to_memory(unsigned short index, unsigned char a);

#endif
