#ifndef __ASM_H
#define __ASM_H

#include "z80-global" 

extern bool WARNINGS;
extern bool LISTING;
extern unsigned short highest_address(void);
extern int generated_bytes(void);
extern void set_compile_pass(unsigned pass);
extern unsigned compile_pass(void);
extern bool disable_pseudo;
extern unsigned check_cond_nesting(void);

extern void out(char a);
extern void set_start_address(unsigned short addr);
extern unsigned short get_current_address(void);
extern unsigned compile(const char *txt); /* return value: 0=OK, 1=error */
extern void asm_init(unsigned char fill); /* this must be run before all */
extern void asm_close(void); /* this must be run after all */

#endif
