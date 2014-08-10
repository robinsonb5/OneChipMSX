#include "z80-decode.h"

word  PC, SP, IX, IY;
byte  cpu_reg[20];
byte  memory[1<<16];
tri   IM;
bit   IFF1, IFF2;
pin   halt, m1;
unsigned  bsw=0, asw=0;
long unsigned  ticks;
long unsigned  m_cycles;


void reset()
{
   PC=0;
   IFF1=IFF2= 0;
   IM=0;  I=0;
   SP=0xffff;
   *A= 0xff;
   *F= 0xff;
   ticks=3;
   m_cycles=1;
}


void dump_reg(unsigned i, char* name)
{
    prirntf("%2s=%02x ",name,cpu_reg[i]);
}

void dump_reg16(word v, char* name)
{
    prirntf("%2s=%04x ",name,v);
}

void dump_cpu()
{
   dump_reg(0,"B");
   dump_reg(1,"C");
   dump_reg(2,"D");
   dump_reg(3,"E");
   dump_reg(4,"H");
   dump_reg(5,"L");
   dump_reg(6,"X");
   dump_reg(7,"A");
   dump_reg(8,"F");
   printf("\n");
   dump_reg(9+0,"B'");
   dump_reg(9+1,"C'");
   dump_reg(9+2,"D'");
   dump_reg(9+3,"E'");
   dump_reg(9+4,"H'");
   dump_reg(9+5,"L'");
   dump_reg(9+6,"X'");
   dump_reg(9+7,"A'");
   dump_reg(9+8,"F'");
   printf("\n");
   dump_reg(18,"I'");
   dump_reg(19,"R'");
   dump_reg16(IX,"IX");
   dump_reg16(IY,"IY");
   dump_reg16(SP,"SP");
   dump_reg16(PC,"PC");
   printf("\n");
}

void  end_cycle(unsigned t)
{
   ticks += t;
   dump_cpu();
}


void  cycle(unsigned t)
{
   ticks += t;
   m_cycles++;
}


byte fetch_next_byte(void)
{
   cycle(3);
   return  memory[PC++];
}


byte fetch_next_word(void)
{
   return  fetch_next_byte() | fetch_next_byte()<<8 ;
}


split332(byte curr, byte *low, tri *mid, byte* hig)
{
  *low = curr & (1<<3)-1;
  *mid = curr>>3 & (1<<3)-1;
  *hig = curr>>6 & (1<<2)-1;
}


void decode(void)
{
  byte  hig, low;
  tri   mid, cond;

  split332(fetch_next_byte(),&low,&mid,&hig);
  switch (hig)
  {
      case 0:   if (low == 4)
                   inc(reg8(mid));
                else if (low == 5)
                   dec(reg8(mid));
                else if (low == 6)
                   ldb(reg8(mid),fetch_next_byte());
                else
                {  low |= (mid&1)<<3;
                   mid >>= 1;
                   switch (low)
                   {
                      case  1:  ldw(reg16(mid),fetch_next_word());
                                break;
                      case  3:  incw(reg16(mid));
                                break;
                      case  9:  addhl(reg16(mid));
                                break;
                      case 11:  decw(reg16(mid));
                                break;
                      case  2:  if (mid == 3)
                                   load(memory+fetch_next_word(),A);
                                else if (mid == 2)
                                   load16((word*)(memory+fetch_next_word()),HL);
                                else
                                   load(memory+*reg16(mid),A);
                                break;
                      case 10:  if (mid == 3)
                                   load(A,memory+fetch_next_word());
                                else if (mid == 2)
                                   load16(HL,(word*)(memory+fetch_next_word()));
                                else
                                   load(A,memory+*reg16(mid));
                                break;
                      default:  cond=4;
                                switch(low>>2 | mid<<2)
                                {
                                   case  0:  nop();   break;
                                   case  1:  rlca(A); break;
                                   case  2:  ex_af(); break;
                                   case  3:  rrca(A); break;
                                   case  4:  djnz(fetch_next_byte());   break;
                                   case  5:  rla(A);  break;
                                   case  6:  jr(fetch_next_byte());   break;
                                   case  7:  rra(A);  break;
                                   case  9:  daa(A);  break;
                                   case 11:  cpl(A);  break;
                                   case 13:  scf();   break;
                                   case 15:  ccf();   break;
                                   case  8:  cond--;
                                   case 10:  cond--;
                                   case 12:  cond--;
                                   case 14:  cond--;
                                             jrF(cond,fetch_next_byte());
                                             break;
                                }
                   }
                }
                break;

      case 1:   if (mid == 6 && low == 6)
                   halt();
                else
                   load(reg8(mid),reg8(low));
                break;

      case 2:   switch (mid)
                {
                    case 0:  add(A,reg8(low)); break;
                    case 1:  adc(A,reg8(low)); break;
                    case 2:  sub(A,reg8(low)); break;
                    case 3:  sbc(A,reg8(low)); break;
                    case 4:  and(A,reg8(low)); break;
                    case 5:  xor(A,reg8(low)); break;
                    case 6:   or(A,reg8(low)); break;
                    case 7:   cp(A,reg8(low)); break;
                }
                break;
      case 3:   if (low == 0)
                   retF(mid);
                else if (low == 2)
                   jpF(mid,fetch_next_word());
                else if (low == 4)
                   callF(mid,fetch_next_word());
                else if (low == 7)
                   rst(mid);
                else if (low == 6)
                {  cpu_reg[6] = fetch_next_byte();
                   switch (mid)
                   {
                      case  0:  add(A,X);   break;
                      case  1:  adc(A,X);   break;
                      case  2:  sub(A,X);   break;
                      case  3:  sbc(A,X);   break;
                      case  4:  and(A,X);   break;
                      case  5:  xor(A,X);   break;
                      case  6:   or(A,X);   break;
                      case  7:   cp(A,X);   break;
                   }
                }
                else
                {  low |= (mid&1)<<8;
                   mid >>= 1;
                   switch (low)
                   {
                      case  1:  pop(reg88(mid));   break;
                      case  5:  push(reg88(mid));   break;
                      case  3:  switch(mid)
                                {
                                   case  0:  jp(fetch_next_word());   break;
                                   case  1:  out(*A<<8|fetch_next_byte(),A);  break;
                                   case  2:  ex_sp_hl();  break;
                                   case  3:  di();  break;
                                }
                                break;
                      case 11:  switch(mid)
                                {
                                   case  0:  decode_CB();   break;
                                   case  1:  in(A,*A<<8|fetch_next_byte());  break;
                                   case  2:  ex_de_hl();  break;
                                   case  3:  ei();  break;
                                }
                                break;
                      case  9:  switch(mid)
                                {
                                   case  0:  ret();   break;
                                   case  1:  exx();  break;
                                   case  2:  jp(*HL);  break;
                                   case  3:  load16(&SP,HL);  break;
                                }
                                break;
                      case 13:  switch(mid)
                                {
                                   case  0:  call(fetch_next_word());  break;
                                   case  1:  decode_DD();  break;
                                   case  2:  decode_ED();  break;
                                   case  3:  decode_FD();  break;
                                }
                                break;
                   }
                }
                break;
  }

}


void decode_CB(void)
{
  byte  hig;
  tri  low, mid;

  split332(fetch_next_byte(),&low,&mid,&hig);

  switch (hig)
  {
      case  0:  switch (mid)
                {
                   case 0:   rlc(reg8(low));   break;
                   case 1:   rrc(reg8(low));   break;
                   case 2:   rl(reg8(low));   break;
                   case 3:   rr(reg8(low));   break;
                   case 4:   sla(reg8(low));   break;
                   case 5:   sra(reg8(low));   break;
                   case 6:   sll(reg8(low));   break;
                   case 7:   srl(reg8(low));   break;
                }
                break;
      case 1:   bit(mid,reg8(low));  break;
      case 2:   res(mid,reg8(low));  break;
      case 3:   set(mid,reg8(low));  break;
  }
}


void decode_ED(void)
{
  byte  hig;
  tri  low, mid;

  split332(fetch_next_byte(),&low,&mid,&hig);

  if (hig==1)
  {
     switch (low)
     {
        case 0:  in((mid==6?X:reg8(mid)),*BC);  break;
        case 1:  out(*BC,(mid==6?0:*reg8(mid)));  break;
        case 2:  if (mid&4) adc16(HL,reg16(mid&3)); else sbc16(HL,reg16(mid&3));
                 break;
        case 3:  if (mid&4) ld16(reg16(mid&3),memory+fetch_next_word());
                 else ld16(memory+fetch_next_word(),reg16(mid&3));
                 break;
        case 4:  neg(A);  break;
        case 5:  if (mid&1) reti() else retn();  break;
        case 6:  im(mid&3); break;
        case 7:  switch (mid)
                 {
                     case 0:  load(I,A);  break;
                     case 1:  load(R,A);  break;
                     case 2:  load(A,I);  break;
                     case 3:  load(A,R);  break;
                     case 4:  rrd();   break;
                     case 5:  rld();   break;
                     case 6:  nop2();  break;
                     case 7:  nop2();  break;
                 }
     }
  }
  else if (hig==2 && mid >= 4 && low <= 3)
  {
      switch (low)
      {
          case 0:  ld_block(mid&3);   break;
          case 1:  cp_block(mid&3);   break;
          case 2:  in_block(mid&3);   break;
          case 3:  out_block(mid&3);  break;
      }
  }
  else
      nop2();
}
