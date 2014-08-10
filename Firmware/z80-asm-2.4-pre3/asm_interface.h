#ifndef  __ASM_INTERFACE
#define  __ASM_INTERFACE

extern unsigned char write_to_memory(unsigned short index, unsigned char a);
extern void error(int line_number,const char * line_string_from_input,char *error_message);

#endif
