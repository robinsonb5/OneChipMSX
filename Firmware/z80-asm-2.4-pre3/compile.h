/* ASSEMBLING FUNCTIONS */

#ifndef __EXEC_H
#define __EXEC_H

extern int c_adc_sbc(int first,int type,int second,int offset,int d);
/* adc and sbc instructions */
/* offset: code of adc(sbc) A,B */
/* d: x in adc(sbc) HL,..  ED 4x */
/* return value: 0=OK, 1=invalid argument, 2=value out of range,3 offset out of r.  */


extern int c_add(int first,int type,int second);
/* return value: 0=OK, 1=invalid argument, 2=value out of range,3=offset out of range */


extern int c_logical(int type,int value,int offset);
/* logical instructions: and,or,xor and instructions cp,sub */
/* offset: code of and(or,....) B */ 
/* return value: 0=OK, 1=invalid argument, 2=value out of range */


extern int c_ex(int t1,int v1,int v2);


extern void c_im(int v);


extern int c_in(int v1,int t2,int v2);
/* return value: 0=OK, 1=invalid argument, 2=value out of range */


extern int c_inc_dec(int type,int value,int offs1,int offs2);
/* instructions inc and dec */
/* offs1: code of inc(dec) B */
/* offs2: code of inc(dec) BC */
/* return value: 0=OK, 1=invalid argument, 2=value out of range */


extern int c_out(int t1,int v1,int v2);
/* return value: 0=OK, 1=invalid argument, 2=value out of range */


extern int c_push_pop(int v,int offset);
/* instructions for push & pop */
/* offset: x in push(pop) AF   0xfx */


extern int c_ret(int value);


extern int c_rst(int value);


extern int c_bit(int b,int type,int value,int offset);
/* bit operations: set,res,bit */
/* offset: code of set(res,bit) x,B */
/* return value: 0=OK, 1=invalid argument, 2=value out of range */


extern int c_shift_rot(int type,int value,int offset);
/* shift and rotation instructions */
/* rr,rl,rrc,rlc,sla,sra,srl,sll */
/* offset: code of rr(rl,...) B */
/* return value: 0=OK, 1=invalid argument, 2=value out of range */


extern int c_jp(int t1,int v1,int t2,int v2);


extern int c_call(int t1,int v1,int t2,int v2);


extern int c_djnz(int t1,int v1);
/* return value: 0=OK, 1=invalid argument, 2=value out of range */


extern int c_jr(int t1,int v1,int t2,int v2);
/* return value: 0=OK, 1=invalid argument, 2=value out of range */


extern int c_ld(int t1,int v1,int t2,int v2);

#endif
