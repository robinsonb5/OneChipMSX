/* REGISTER AND FLAG DEFINITIONS FOR LEXICAL ANALYSIS (PART OF ASSEMBLING) */

#ifndef __REGS_H
#define __REGS_H

#include "z80-global"
#include "regs_token"

extern unsigned char* const reg_adr[N_8BIT_REGS]; /* 8-bit register address */
extern const char* reg_name[N_REGISTERS+1];  /* regist used for disassembling */
extern const char* flag_name[N_FLAGS+1];  /* flags used for disassembling */
extern const struct seznam_type reg[N_REGISTERS]; /* registers used in assemb */

extern const struct seznam_type flag[N_FLAGS]; /* flags used in instructions */

#endif
