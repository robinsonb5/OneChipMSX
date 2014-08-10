/* REGISTER AND FLAG TABLES FOR LEXICAL ANALYSIS (PART OF ASSEMBLING) */

#include "z80-cpu.h"
#include "regs_token"

_uchar * const reg_adr[N_8BIT_REGS]={&A,&B,&C,&D,&E,&H,&L,&IXh,&IXl,&IYh,&IYl,&I,&R};
/** the order of the names in reg_name[] must match the order of the R_* defines in regs_token !! **/
const char *reg_name[N_REGISTERS+1]={"","A","B","C","D","E","H","L",
     "IXh","IXl","IYh","IYl","I","R","AF","BC","DE","HL","SP","IX","IY","AF'"};

const struct seznam_type reg[N_REGISTERS]={  /* sorted lexicographically */ 
{"A", R_A},
{"AF", R_AF},
{"AF'", R_AF_},
{"B", R_B},
{"BC", R_BC},
{"C", R_C},
{"D", R_D},
{"DE", R_DE},
{"E", R_E},
{"H", R_H},
{"HL", R_HL},
{"I", R_I},
{"IX", R_IX},
{"IXh", R_IXh},
{"IXl", R_IXl},
{"IY", R_IY},
{"IYh", R_IYh},
{"IYl", R_IYl},
{"L", R_L},
{"R", R_R},
{"SP", R_SP}
};

/** the order of the names in flag_name[] must match the order of the F_* defines in regs_token !! **/
const char *flag_name[N_FLAGS+1]={"","Z","NC","C","PO","PE","P","M","NZ"};

const struct seznam_type flag[N_FLAGS]={ /* sorted lexicographically */
{"C", F_C},
{"M", F_M},
{"NC", F_NC},
{"NZ", F_NZ},
{"P", F_P},
{"PE", F_PE},
{"PO", F_PO},
{"Z", F_Z}
};
