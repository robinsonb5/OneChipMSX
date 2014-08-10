typedef  unsigned char   pin;
typedef  unsigned char   byte;
typedef  unsigned char   tri;  /* only values < 8 valid */
typedef  unsigned short  word;

#define  B  reg8(0)
#define  C  reg8(1)
#define  D  reg8(2)
#define  E  reg8(3)
#define  H  reg8(4)
#define  L  reg8(5)
#define  X  (cpu_reg+6)
#define  A  reg8(7)
#define  F  reg8(8)
#define  reg8(r)  ( (r)==6 ? memory+ *HL: cpu_reg+((r)>=7?asw:bsw)+(r) )
#define  BC  reg16(0)
#define  DE  reg16(1)
#define  HL  reg16(2)
#define  reg16(r)  ((r) != 3 ? (word*)(cpu_reg+bsw+2*(r)) : &SP)
#define  reg88(r)   (word*)(cpu_reg+(r==3?asw:bsw)+2*(r)+(r==3))
#define  I   (cpu_reg+18)
#define  R   (cpu_reg+19)


void inc(byte*);
void dec(byte*);
void ldb(byte*,byte);
void ldw(word*,word);
void incw(word*);
void decw(word*);
void addhl(word*);

void load(byte*,byte*);
void load16(word*,word*);

void add(byte*,byte*);
void adc(byte*,byte*);
void sub(byte*,byte*);
void sbc(byte*,byte*);
void and(byte*,byte*);
void xor(byte*,byte*);
void  or(byte*,byte*);
void  cp(byte*,byte*);

void halt();

void nop(void);
void rlca(byte*);
void rrca(byte*);
void rla(byte*);
void rra(byte*);
void ex_af(void);
void djnz(byte);
void jr(byte);
void daa(byte*);
void cpl(byte*);
void scf(void);
void ccf(void);
void jrF(tri,byte);
void retF(tri);
void jpF(tri,word);
void callF(tri,word);
void rst(tri);
void pop(word*);
void push(word*);
void jp(word);
void out(word,byte*);
void in(byte*,word);
void ex_sp_hl(void);
void ex_de_hl(void);
void ei(void);
void di(void);
void ret(void);
void exx(void);
void call(word);

void decode_CB(void);
void decode_DD(void);
void decode_ED(void);
void decode_FD(void);

void rlc(byte*);
void rrc(byte*);
void rl(byte*);
void rr(byte*);
void sla(byte*);
void sra(byte*);
void sll(byte*);
void srl(byte*);
void bit(tri,byte*);
void res(tri,byte*);
void set(tri,byte*);

void in(byte*,byte);
void out(byte*,byte*);
void adc16(word*,word*);
void sbc16(word*,word*);
void ld16(word*,word*);
void neg(byte*);
void retn(void);
void im(tri);  / tri==0 tri==1  --> 0    tri==2 --> 1  tri==3 --> 2 */
void rld(void);
void rrd(void);
void ld_block(tri);
void cp_block(tri);
void in_block(tri);
void out_block(tri);
void nop2(void);
