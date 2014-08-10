/* ASSEMBLING FUNCTIONS  */

#include "execute_token"
#include "regs.h"
#include "asm.h"

#include <stdio.h>

/* adc and sbc instructions */
/* offset: code of adc(sbc) A,B */
/* d: x in adc(sbc) HL,..  ED 4x */
/* return value: 0=OK, 1-7=invalid argument, 8=value out of range,9 offset out of r.  */

int 
c_adc_sbc(int first,int type,int second,int offset,int d)
{
if (first==R_A)
 {
 if (type==A_PODLE_REG&&second==R_HL){out(offset+6);return 0;}
 if (type==A_NUM)
  {
  out(offset+70);out(second);
  return second>255 ? 8 : 0;
  }
 if (type==A_PODLE_IX_PLUS)
  {
  out(0xdd);out(offset+6);out(second);
  if (second>127||second<-128) return 9;
  return 0;
  }
 if (type==A_PODLE_IY_PLUS)
  {
  out(0xfd);out(offset+6);out(second);
  if (second>127||second<-128)return 9;
  return 0;
  }
 }

if (type==A_REG)
 {
 switch (first)
  {
  case R_A:
  switch (second)
   {
   case R_A:
   out(offset+7);
   break;
   case R_B:
   out(offset);
   break;
   case R_C:
   out(offset+1);
   break;
   case R_D:
   out(offset+2);
   break;
   case R_E:
   out(offset+3);
   break;
   case R_H:
   out(offset+4);
   break;
   case R_L:
   out(offset+5);
   break;
   default:
   return 1;
   }
  break;
 
  case R_HL:
  switch (second)
   {
   case R_BC:
   out(0xed);out(0x40+d);
   break;
   case R_DE:
   out(0xed);out(0x50+d);
   break;
   case R_HL:
   out(0xed);out(0x60+d);
   break;
   case R_SP:
   out(0xed);out(0x70+d);
   break;
   default:
   return 1;
   }
  break;

  default:
  return 1;
  }
 return 0;
 }
return 1;
}


/* return value: 0=OK, 1=invalid argument, 8=value out of range,9=offset out of range */

int 
c_add(int first,int type,int second)
{
if (first==R_A)
 {
 if (type==A_PODLE_REG&&second==R_HL){out(0x86);return 0;}
 if (type==A_NUM)
  {
  out(0xc6);out(second);
  return second>255 ? 8 : 0;
  }
 if (type==A_PODLE_IX_PLUS)
  {
  out(0xdd);out(0x86);out(second);
  if (second>127||second<-128)return 9;
  return 0;
  }
 if (type==A_PODLE_IY_PLUS)
  {
  out(0xfd);out(0x86);out(second);
  if (second>127||second<-128)return 9;
  return 0;
  }
 }

if (type==A_REG)
 {
 switch (first)
  {
  case R_A:
  switch (second)
   {
   case R_A:
   out(0x87);
   break;
   case R_B:
   out(0x80);
   break;
   case R_C:
   out(0x81);
   break;
   case R_D:
   out(0x82);
   break;
   case R_E:
   out(0x83);
   break;
   case R_H:
   out(0x84);
   break;
   case R_L:
   out(0x85);
   break;
   default:
   return 1;
   }
  break;
 
  case R_HL:
  switch (second)
   {
   case R_BC:
   out(0x09);
   break;
   case R_DE:
   out(0x19);
   break;
   case R_HL:
   out(0x29);
   break;
   case R_SP:
   out(0x39);
   break;
   default:
   return 1;
   }
  break;
  
  case R_IX:
  switch (second)
   {
   case R_BC:
   out(0xdd);out(0x09);
   break;
   case R_DE:
   out(0xdd);out(0x19);
   break;
   case R_IX:
   out(0xdd);out(0x29);
   break;
   case R_SP:
   out(0xdd);out(0x39);
   break;
   default:
   return 1;
   }
  break;
  
  case R_IY:
  switch (second)
   {
   case R_BC:
   out(0xfd);out(0x09);
   break;
   case R_DE:
   out(0xfd);out(0x19);
   break;
   case R_IY:
   out(0xfd);out(0x29);
   break;
   case R_SP:
   out(0xfd);out(0x39);
   break;
   default:
   return 1;
   }
  break;

  default:
  return 1;
  }
 return 0;
 }
return 1;
}


/* logical instructions: and,or,xor and instructions cp,sub */
/* offset: code of and(or,....) B */ 
/* return value: 0=OK, 1=invalid argument, 8=value out of range */

int 
c_logical(int type,int value,int offset)
{
switch(type)
 {
 case A_REG:
 switch(value)
  {
  case R_A:
  out(offset+7);
  break;
  case R_B:
  out(offset);
  break;
  case R_C:
  out(offset+1);
  break;
  case R_D:
  out(offset+2);
  break;
  case R_E:
  out(offset+3);
  break;
  case R_H:
  out(offset+4);
  break;
  case R_L:
  out(offset+5);
  break;
  default:
  return 1;
  }
 break;
 
 case A_PODLE_REG:
 if (value!=R_HL) return 1;
 out(offset+6);
 break;
 
 case A_PODLE_IX_PLUS:
 out(0xdd);out(offset+6);out(value);
 if (value>127||value<-128)return 9;
 break;
 
 case A_PODLE_IY_PLUS:
 out(0xfd);out(offset+6);out(value);
 if (value>127||value<-128)return 9;
 break;
 
 case A_NUM:
 out(0x46+offset);out(value);
 if (value>255) return 8;
 break;
 
 default:
 return 1; 
 }
return 0;
}


int 
c_ex(int t1,int v1,int v2)
{
switch(t1)
 {
 case A_REG:
 if (v1==R_DE&&v2==R_HL){out(0xeb);return 0;}
 if (v1==R_AF&&v2==R_AF_){out(0x08);return 0;}
 return 1;
 
 case A_PODLE_REG:
 if (v1!=R_SP)return 1;
 switch (v2)
 {
  case R_HL:
  out(0xe3);
  break;
  case R_IX:
  out(0xdd);out(0xe3);
  break;
  case R_IY:
  out(0xfd);out(0xe3);
  break;
  default:
  return 1;
 }
 break;

 default:
 return 1;
 }
return 0;
}


void 
c_im(int v)
{
out(0xed);
switch (v)
 {
 case 0:
 out(0x46);
 break;
 
 case 1:
 out(0x56);
 break;
 
 case 2:
 out(0x5e);
 break;
 }
}


/* return value: 0=OK, 1=invalid argument, 8=value out of range */

int 
c_in(int v1,int t2,int v2)
{
if (t2==A_PODLE_NUM)
 {
 if (v1==R_A)
 {  out(0xdb);  out(v2);
    return  v2>255 ? 8 : 0 ;
 }
 else return 1;
 }
if (t2==A_PODLE_REG&&v2==R_C)
 {
 switch(v1)
  {
  case R_A:
  out(0xed);out(0x78);
  break;
  case R_B:
  out(0xed);out(0x40);
  break;
  case R_C:
  out(0xed);out(0x48);
  break;
  case R_D:
  out(0xed);out(0x50);
  break;
  case R_E:
  out(0xed);out(0x58);
  break;
  case R_H:
  out(0xed);out(0x60);
  break;
  case R_L:
  out(0xed);out(0x68);
  break;
  default:
  return 1;
  }
 return 0;
 }
return 1;
}


/* instructions inc and dec */
/* offs1: code of inc(dec) B */
/* offs2: code of inc(dec) BC */
/* return value: 0=OK, 1=invalid argument, 8=value out of range */

int 
c_inc_dec(int type,int value,int offs1,int offs2)
{
switch(type)
 {
 case A_REG:
 switch(value)
  {
  case R_A:
  out(0x38+offs1);
  break;
  case R_B:
  out(offs1);
  break;
  case R_C:
  out(0x08+offs1);
  break;
  case R_D:
  out(0x10+offs1);
  break;
  case R_E:
  out(0x18+offs1);
  break;
  case R_H:
  out(0x20+offs1);
  break;
  case R_L:
  out(0x28+offs1);
  break;
  case R_BC:
  out(offs2);
  break;
  case R_DE:
  out(0x10+offs2);
  break;
  case R_HL:
  out(0x20+offs2);
  break;
  case R_IX:
  out(0xdd);out(0x20+offs2);
  break;
  case R_IY:
  out(0xfd);out(0x20+offs2);
  break;
  case R_SP:
  out(0x30+offs2);
  break;
  default:
  return 1;
  }
 break;
 
 case A_PODLE_REG:
 if (value!=R_HL) return 1;
 out(0x30+offs1);
 break;
 
 case A_PODLE_IX_PLUS:
 out(0xdd);out(0x30+offs1);out(value);
 if (value>127||value<-128)return 9;
 break;
 
 case A_PODLE_IY_PLUS:
 out(0xfd);out(0x30+offs1);out(value);
 if (value>127||value<-128)return 9;
 break;
 
 default:
 return 1; 
 }
return 0;
}


/* return value: 0=OK, 1=invalid argument, 8=value out of range */

int 
c_out(int t1,int v1,int v2)
{
if (t1==A_PODLE_NUM)
 {
 if (v2==R_A)
 {  out(0xd3);out(v1);
    return  v1>255 ? 8 : 0 ;
 }
 else return 1;
 }
if (t1==A_PODLE_REG&&v1==R_C)
 {
 switch(v2)
  {
  case R_A:
  out(0xed);out(0x79);
  break;
  case R_B:
  out(0xed);out(0x41);
  break;
  case R_C:
  out(0xed);out(0x49);
  break;
  case R_D:
  out(0xed);out(0x51);
  break;
  case R_E:
  out(0xed);out(0x59);
  break;
  case R_H:
  out(0xed);out(0x61);
  break;
  case R_L:
  out(0xed);out(0x69);
  break;
  default:
  return 1;
  }
 return 0;
 }
return 1;
}


/* instructions for push & pop */
/* offset: x in push(pop) AF   0xfx */

int 
c_push_pop(int v,int offset)
{
switch (v)
 {
 case R_AF:
 out(0xf0+offset);
 break;
 
 case R_BC:
 out(offset+0xc0);
 break;
 
 case R_DE:
 out(offset+0xd0);
 break;
 
 case R_HL:
 out(offset+0xe0);
 break;
 
 case R_IX:
 out(0xdd);out(offset+0xe0);
 break;
 
 case R_IY:
 out(0xfd);out(offset+0xe0);
 break;
 
 default:
 return 1;
 }
return 0;
}


int 
c_ret(int value)
{
switch (value)
 {
 case F_NZ:
 out(0xc0);
 break;
 case F_Z:
 out(0xc8);
 break;
 case F_NC:
 out(0xd0);
 break;
 case F_C:
 out(0xd8);
 break;
 case F_PO:
 out(0xe0);
 break;
 case F_PE:
 out(0xe8);
 break;
 case F_P:
 out(0xf0);
 break;
 case F_M:
 out(0xf8);
 break;
 default:
 return 1;
 }
return 0;
}


int 
c_rst(int value)
{
switch(value)
 {
 case 0:
 out(0xc7);
 break;
 
 case 8:
 out(0xcf);
 break;
 
 case 0x10:
 out(0xd7);
 break;
 
 case 0x18:
 out(0xdf);
 break;
 
 case 0x20:
 out(0xe7);
 break;
 
 case 0x28:
 out(0xef);
 break;
 
 case 0x30:
 out(0xf7);
 break;
 
 case 0x38:
 out(0xff);
 break;
 
 default:
 return 1;
 }
return 0;
}


/* bit operations: set,res,bit */
/* offset: code of set(res,bit) x,B */
/* return value: 0=OK, 1=invalid argument, 8=value out of range */

int 
c_bit(int b,int type,int value,int offset)
{
switch(type)
 {
 case A_REG:
 switch(value)
  {
  case R_A:
  out(0xcb);out(offset+7+(b<<3));
  break;
  case R_B:
  out(0xcb);out(offset+(b<<3));
  break;
  case R_C:
  out(0xcb);out(offset+1+(b<<3));
  break;
  case R_D:
  out(0xcb);out(offset+2+(b<<3));
  break;
  case R_E:
  out(0xcb);out(offset+3+(b<<3));
  break;
  case R_H:
  out(0xcb);out(offset+4+(b<<3));
  break;
  case R_L:
  out(0xcb);out(offset+5+(b<<3));
  break;
  default:
  return 1;
  }
 break;
 
 case A_PODLE_REG:
 if (value!=R_HL)return 1;
 out(0xcb);out(offset+6+(b<<3));
 break;

 case A_PODLE_IX_PLUS:
 out(0xdd);out(0xcb);out(value);out(offset+6+(b<<3));
 if (value>127||value<-128)return 9;
 break;

 case A_PODLE_IY_PLUS:
 out(0xfd);out(0xcb);out(value);out(offset+6+(b<<3));
 if (value>127||value<-128)return 9;
 break;
 
 default:
 return 1;
 }
return 0;
}


/* shift and rotation instructions */
/* rr,rl,rrc,rlc,sla,sra,srl,sll */
/* offset: code of rr(rl,...) B */
/* return value: 0=OK, 1=invalid argument, 8=value out of range */

int
c_shift_rot(int type,int value,int offset)
{
switch(type)
 {
 case A_REG:
 switch(value)
  {
  case R_A:
  out(0xcb);out(offset+7);
  break;
  case R_B:
  out(0xcb);out(offset);
  break;
  case R_C:
  out(0xcb);out(offset+1);
  break;
  case R_D:
  out(0xcb);out(offset+2);
  break;
  case R_E:
  out(0xcb);out(offset+3);
  break;
  case R_H:
  out(0xcb);out(offset+4);
  break;
  case R_L:
  out(0xcb);out(offset+5);
  break;
  default:
  return 1;
  }
 break;
 
 case A_PODLE_REG:
 if (value!=R_HL) return 1;
 out(0xcb);out(offset+6);
 break;
 
 case A_PODLE_IX_PLUS:
 out(0xdd);out(0xcb);out(value);out(offset+6);
 if (value>127||value<-128)return 9;
 break;
 
 case A_PODLE_IY_PLUS:
 out(0xfd);out(0xcb);out(value);out(offset+6);
 if (value>127||value<-128)return 9;
 break;
 
 default:
 return 1; 
 }
return 0;
}


int
c_jp(int t1,int v1,int t2,int v2)
{
if (t2==A_EMPTY)
 switch (t1)
  {
  case A_REG:
  case A_PODLE_REG:
  switch(v1)
   {
   case R_HL:
   out(0xe9);
   break;
   case R_IX:
   out(0xdd);out(0xe9);
   break;
   case R_IY:
   out(0xfd);out(0xe9);
   break;
   default:
   return 1;
   }
  break;
 
  case A_NUM:
  out (0xc3);out(v1&255);out((v1>>8)&255);
  break;

  default:
  return 1;
  }
else 
 {
 if (t1==A_REG)
 {
  if (v1!=R_C)return 1;
 }
 else {if (t1!=A_FLAG)return 1;}
 if (t2!=A_NUM) return 1;
 switch (v1)
  {
  case F_NZ:
  out(0xc2);
  break;
  case F_Z:
  out(0xca);
  break;
  case F_NC:
  out(0xd2);
  break;
  case F_C:
  out(0xda);
  break;
  case F_PO:
  out(0xe2);
  break;
  case F_PE:
  out(0xea);
  break;
  case F_P:
  out(0xf2);
  break;
  case F_M:
  out(0xfa);
  break;
  default:
  return 1;
  }
 out(v2&255);
 out((v2>>8)&255);
 }
return 0;
}
 

int
c_call(int t1,int v1,int t2,int v2)
{
if (t2==A_EMPTY)
 switch (t1)
  {
  case A_NUM:
  out (0xcd);out(v1&255);out((v1>>8)&255);
  break;

  default:
  return 1;
  }

else 
 {
 if (t1==A_REG)
 {
  if (v1!=R_C)return 1;
 }
 else {if (t1!=A_FLAG)return 1;}
 if (t2!=A_NUM) return 1;
 switch (v1)
  {
  case F_NZ:
  out(0xc4);
  break;
  case F_Z:
  out(0xcc);
  break;
  case F_NC:
  out(0xd4);
  break;
  case F_C:
  out(0xdc);
  break;
  case F_PO:
  out(0xe4);
  break;
  case F_PE:
  out(0xec);
  break;
  case F_P:
  out(0xf4);
  break;
  case F_M:
  out(0xfc);
  break;
  default:
  return 1;
  }
 out(v2&255);
 out((v2>>8)&255);
 }
return 0;
}
 

/* return value: 0=OK, 1=invalid argument, 8=value out of range */

int
c_djnz(int t1,int v1)
{
int a;

switch (t1)
 {
 case A_NUM:
 a=v1;
 break;
 
 default:
 return 1;
 }
out(0x10);
out ((char)a);
if (a>127||a<-128)return 9;
return 0;
}


/* return value: 0=OK, 1=invalid argument, 8=value out of range */

int
c_jr(int t1,int v1,int t2,int v2)
{
int a,c;

if (t2==A_EMPTY)
 {
 switch (t1)
  {
  case A_NUM:
  a=v1;
  break;
  
  default:
  return 1;
  }
 out(0x18);
 out ((char)a);
 if (a>127||a<-128) return 9;
 }
else
 {
 if (t1==A_REG)
 {
  if (v1!=R_C)return 1;
 }
 else {if (t1!=A_FLAG)return 1;}
 switch(v1)
  {
  case F_NZ:
  c=0x20;
  break;
  case F_Z:
  c=0x28;
  break;
  case F_NC:
  c=0x30;
  break;
  case F_C:
  c=0x38;
  break;
  default:
  return 1;
  }
 switch (t2)
  {
  case A_NUM:
  a=v2;
  break;
  
  default:
  return 1;
  }
 out(c);
 out ((char)a);
 if (a>127||a<-128) return 9;
 }
return 0;
}


int
reg_num(int reg)
{
switch(reg)
 {
 case R_A:
 return 7;
 case R_B:
 return 0;
 case R_C:
 return 1;
 case R_D:
 return 2;
 case R_E:
 return 3;
 case R_H:
 return 4;
 case R_L:
 return 5;
 default:
 return 16;
 }
}


int
c_ld(int t1,int v1,int t2,int v2)
{
int a=0,b=0;
switch (t1)
 {
 case A_REG:   /* ld 8b-reg,?? */
 if (v1==R_A||v1==R_B||v1==R_C||v1==R_D||v1==R_E||v1==R_H||v1==R_L||v1==R_I||v1==R_R)
 switch (t2)
  {
  case A_REG:           /* ld 8b-reg,8b-reg */
  if (v1==R_A&&v2==R_I){out(0xed);out(0x57);return 0;}
  if (v1==R_A&&v2==R_R){out(0xed);out(0x5f);return 0;}
  if (v2==R_A&&v1==R_I){out(0xed);out(0x47);return 0;}
  if (v2==R_A&&v1==R_R){out(0xed);out(0x4f);return 0;}
  a=reg_num(v1);
  b=reg_num(v2);
  if (a==16||b==16)return 1;
  out (b+(a<<3)+64);
  return 0;

  case A_NUM:    /* ld 8b-reg,num */
  a=reg_num(v1);
  if (a==16)return 1;
  out (6+(a<<3));out(v2);
  return  v2>255 ? 8 : 0 ;

  case A_PODLE_REG:   /* ld 8b-reg,(16b-reg) */
  if (v1==R_A)
   {
   if (v2==R_BC){out (0x0a);return 0;}
   if (v2==R_DE){out (0x1a);return 0;}
   }
  if (v2!=R_HL)return 1;
  a=reg_num(v1);
  if (a==16)return 1;
  out (70+(a<<3));
  return 0;

  case A_PODLE_NUM:   /* ld 8b-reg,(addr) */
  if (v1!=R_A)return 1;
  out(0x3a);out(v2&255);out((v2>>8)&255);
  return 0;

  case A_PODLE_IY_PLUS:   /* ld 8b-reg,(ix/iy+num) */
  b=0xfd;
  case A_PODLE_IX_PLUS:
  if (!b)b=0xdd;
  a=reg_num(v1);
  if (a==16)return 1;
  out (b);out (70+(a<<3));out(v2);
  if (v2>127||v2<-128) return 9;
  return 0;
  }
    
 switch (v1)    /* ld 16b-reg,XX or ld 16b-reg,(XX) */
  {
  case R_BC:
  a=0;b=0;
  break;
  case R_DE:
  a=0x10;b=0;
  break;
  case R_HL:
  a=0x20;b=0;
  break;
  case R_IX:
  a=0x20;b=0xdd;
  break;
  case R_IY:
  b=0xfd;a=0x20;
  break;
  case R_SP:
  if (t2==A_REG)   /* ld sp,hl/ix/iy */
   {
   if (v2==R_HL){out(0xf9);return 0;}
   if (v2==R_IX){out(0xdd);out(0xf9);return 0;}
   if (v2==R_IY){out(0xfd);out(0xf9);return 0;}
   return 1;
   }
  b=0;a=0x30;
  break;
  }
 switch (t2)
  {
  case A_NUM:
  if (b)out(b);
  out(a+1);out(v2&255);out((v2>>8)&255);
  return 0;

  case A_PODLE_NUM:
  if (b){out(b);out(0xa+a);}
  else if (a==0x20) {out(0x2a);}
  else {out(0xed);out(0x4b+a);}
  out(v2&255);out((v2>>8)&255);
  return 0;
  }
 return 1; 
 
 case A_PODLE_REG:   /* ld (16b-reg),?? */
 if (v1==R_HL&&t2==A_REG)   /* ld (hl),8b-reg */
  {
  a=reg_num(v2);
  if (a==16)return 1;
  out (0x70+a);
  return 0;
  }
 if (v1==R_HL&&t2==A_NUM)    /* ld (hl),num */
  {
  out(0x36);out(v2);
  return  v2>255 ? 8 : 0 ;
  }
 if (t2==A_REG&&v2==R_A)   /* ld (bc/de),a */
  {
  if (v1==R_BC){out(0x02);return 0;}
  if (v1==R_DE){out(0x12);return 0;}
  return 1;
  }
 return 1;

 case A_PODLE_IX_PLUS:    /* ld (ix+num),?? */
 a=0xdd;
 case A_PODLE_IY_PLUS:
 if (!a)a=0xfd;
 if (t2==A_REG)     /* ld (ix/iy+num),8b-reg */
  {
  b=reg_num(v2);
  if (b==16)return 1;
  out (a);out(0x70+b);out(v1);
  if (v1>127||v1<-128) return 9;
  return 0;
  }
 if (t2==A_NUM)   /* ld (ix/iy+num),num */
  {
  out (a);out(0x36);out(v1);out(v2);
  if (v1>127||v1<-128) return 9;
  return  v2>255 ? 8 : 0 ;
  }
 return 1;
 
 case A_PODLE_NUM:   /* ld (addr),reg */
 if (t2!=A_REG)return 1;
 switch(v2)
  {
  case R_BC:
  out(0xed);out(0x43);
  break;

  case R_A:
  out(0x32);
  break;

  case R_DE:
  out(0xed);out(0x53);
  break;

  case R_HL:
  out(0x22);
  break;

  case R_IX:
  out(0xdd);out(0x22);
  break;

  case R_IY:
  out(0xfd);out(0x22);
  break;

  case R_SP:
  out (0xed);out(0x73);
  break;
  
  default:
  return 1;
  }
 out(v1&255);
 out((v1>>8)&255);
 return 0;
 }
 return 1;
}
