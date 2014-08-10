/* MAIN ASSEMBLING FUNCTIONS */
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

#include "regs.h"
#include "compile.h"
#include "hash.h"
#include "expression.h"
#include "execute_token"
#include "z80-cpu.h"
#include "instr.h"
#include "asm_interface.h"
#include "asm_token"

#define MAX_TEXT 1024

#define   TAB    '\t'
#define   SPACE  ' '
#define   COLON  ':'
#define   QUOTE  '"'
#define   SINGLE '\''

bool LISTING=0;
bool WARNINGS=0;

bool disable_pseudo=1;
static unsigned short end; /* undefined as long as count=0 */

static unsigned lineno, count, cond_level, last_true_cond_level;
static bool label_access, expect_defl;

struct argument_type
{
 unsigned char type;
 int value;
 char *text;            /* this item contains defm string or equ expression */
 bool is_label;         /* indicates if number was from label for JR & DJNZ */
 bool is_resolved;      /* indicates whether value is well defined */
 struct argument_type *next,*previous;
};

struct lex_type
{
 int instruction;
 struct argument_type *arg;
}lex;

/* pruchod=1: parse label definitions, pruchod=2: evaluate labels in operands */
static int pruchod=0;  /* one line compilation, single pass, no labels permitted */
static unsigned short address, last;
static unsigned char  fill_byte;


#ifdef UNIX

char * 
strupr (char *txt)
{
 char *p;
 for (p=txt;*p;p++)
  *p=toupper(*p);
 return txt;
}

#endif /* UNIX */


void
asm_close(void)
{
 free_hash_table();
}


void
asm_init(unsigned char fill)
{
 hash_table_init();
 count=0;
 address=0;
 fill_byte= fill;
    /* funckin' DJGPP doesn't initialize global variables in modules */
    /* with zero so variable end contains garbage in programs using */
    /* asm.a library */
}


/* binary finds txt in seznam */

static int
convert(const struct seznam_type *seznam,int seznam_size,char *txt)
{
int a,b,begin,end;
begin=0;end=seznam_size-1;

if (!(*txt))return 0;
while (1)
 {
 a=((end-begin)>>1)+begin;
 if (!(b=strcmp(seznam[a].name,txt)))return seznam[a].code;
 if (b>0)end=a-1;
 if (b<0)begin=a+1;
 if (end<begin)return -1;
 }
}


static int
is_indirect(char *txt)
{
int  a, b= txt[0]=='(';
if (!b)  return 0;
for (a=1;txt[a];a++)
   if (!b)  break;
   else if (txt[a]=='(') b++;
   else if (txt[a]==')') b--;
return  !txt[a];
}


static int
convert_arg(struct argument_type *arg,char *txt)
{
int a,l=strlen(txt);

arg->is_label=0;
arg->is_resolved=1;
arg->text=0;

/* nothing */
if (!*txt){arg->type=A_EMPTY;arg->value=0;return NIL;}

/* check for string (number of quotes must already be even) */
if ((*txt)==QUOTE)
{
 arg->type=A_STRING;
 arg->text=malloc(l-1);
 if(!arg->text) return IEM;
 for (l=0,a=1;txt[a];a++)
 {
    if (txt[a]==QUOTE)
    { a++;
      if (txt[a]!=QUOTE)
        break;
    }
    arg->text[l++]=txt[a]; 
 }
 arg->text[l]=0;
 if (txt[a]) return UNQ;
 return NIL;
}

if (*txt != SINGLE) strupr(txt);   /* convert all letters to upper case */
else if (!txt[1] || txt[2]!=SINGLE)
   return UNS;

/* check for register */
a=convert(reg,N_REGISTERS,txt);
if (a!=-1){arg->type=A_REG;arg->value=a;return NIL;}

/* check for flag */
a=convert(flag,N_FLAGS,txt);
if (a!=-1){arg->type=A_FLAG;arg->value=a;return NIL;}

if (txt[0]=='(' && txt[l-1]==')' && is_indirect(txt))
{
   txt=memmove(txt,txt+1,strlen(txt));
   txt[strlen(txt)-1]=0;

   /* (check for register) */
   arg->value=convert(reg,N_REGISTERS,txt);
   if (arg->value!=-1){arg->type=A_PODLE_REG;return NIL;}

   if (txt[0]=='I' && (txt[1]=='Y'||txt[1]=='X') && (txt[2]=='+'||txt[2]=='-'))
   {  /* (IX+num) or (IY+num) or (IX-num) or (IY-num) */
      arg->type= (txt[1]=='Y' ? A_PODLE_IY_PLUS : A_PODLE_IX_PLUS);
      if (test_number(txt+2,&a))
         arg->value=a;
      else
      {  a=parse_expr(txt+2,&arg->value,lineno);
         arg->is_resolved= !a;
         if (a < 0)  return -a;
         if (a > 0 && pruchod!=1)  return LBNO;
         arg->is_label=1;
      }
      return NIL;
   }
   if (!txt[test_number(txt,&a)])
      arg->value=a;
   else
   {  a=parse_expr(txt,&arg->value,lineno);
      arg->is_resolved= !a;
      if (a < 0)  return -a;
      if (a > 0 && pruchod!=1)  return LBNO;
      arg->is_label=1;
   }
   arg->type=A_PODLE_NUM;
   return NIL;
 }
 if (!txt[test_number(txt,&a)])
    arg->value=a;
 else
 {  a=parse_expr(txt,&arg->value,lineno);
    arg->is_resolved= !a;
    if (a < 0)  return -a;
    if (a > 0 && pruchod!=1)  return LBNO;
    if (a > 0)  arg->value=address;  /* value needed for JR/DJNZ/EQU/DEFL/DEFS/ORG */
    arg->is_label=1;
 }
 arg->type=A_NUM;
 return NIL;
}


/* converts line into structure lex */
/* return value:
   0=O.K.
   1=memory allocation error
   2=unknown label
   3=label previously defined
   4=invalid label name
   5=illegal label declaration
   6=illegal operator
   7=unbalanced quotes
   8=missing opening parenthesis
   9=missing closing parenthesis
   10=unbalanced single quotes
   11=token following string
   12=token following pharanthesis
   13=invalid character argument
   14=syntax error in expression
   15=defl expected
   16=expression not pass 1 evaluable
   17=missing label
   18=missing operand
*/

static int
lexical_analysis(const char *line)
{
char txt1[MAX_TEXT];
const char *p;
char *b=NULL;
struct argument_type *a,*c;
int ret;

lex.instruction=I_EMPTY;
lex.arg=NULL;
a=NULL;c=NULL;

if ((*line)==';')return 0;  /* ignore comment */

/* LABEL */

label_access= 0;
expect_defl=0;
if (!pruchod)
 {
 for (p=line;(*p)!=SPACE&&(*p)!=TAB&&(*p);p++)  /* skip label */
    if (!pruchod && *p > SPACE)  return ILD;
 if (!(*p))return 0;
 }
else
 {
 for (p=line,b=txt1;(*p)!=SPACE&&(*p)!=TAB&&(*p);b++,p++)
  *b=toupper(*p);
 if (p!=line && cond_level == last_true_cond_level)
  {
   if (*(b-1)==COLON && b-1 != txt1)  b--;  /* delete one trailing colon */
   *b=0;
   if (b==txt1 || test_label(txt1) != b-txt1)  return ILB;
   if (is_in_table(txt1,0,0,0))
   {  if (pruchod==1)
      {  if (last_label_reusable())
         { 
            if (!(label_access=add_to_table(txt1,address,lineno,1))) return IEM;
            expect_defl=1;
         }
         else
            return LBAR;
      }
      else
         label_access=reaccess_label(txt1,lineno);
   }
   else
   {  if (pruchod==2)  return MISL;
      if (!(label_access=add_to_table(txt1,address,lineno,1))) return IEM;
   }
  }
 }

/* SPACE */

while((*p)==TAB||(*p)==SPACE)  /* skip spaces */
 p++;
if ((*p)==';')return 0;
if (!(*p))return 0;

/* INSTRUCTION */

for (b=txt1;((*p)!=SPACE&&(*p)!=TAB&&(*p));p++,b++)
 *b=toupper(*p);
*b=0;
lex.instruction=convert(instruction,N_INSTRUCTIONS,txt1);
if (expect_defl && lex.instruction != I_DEFL)
   return  DEX;

/* SPACE */

while((*p)==TAB||(*p)==SPACE)
 p++;
if (!(*p) || (*p)==';')
 return 0;

a=(struct argument_type *)malloc(sizeof(struct argument_type));
if (!a) return IEM;
a->next=NULL;
c=a;
if (cond_level > last_true_cond_level){lex.arg=a->next;free(c);return 0;}

/* ARGUMENTS */

while(1)
 {
 int single=0, quotes=0, parent=0;
 /* arguments are separated by ' ' or ','; separators in quotes are ignored */
 /* we stop reading on terminating null character */
 for (b=txt1; (((*p)!=SPACE&&(*p)!=TAB&&(*p!=','))||(quotes&1)||single) && (*p);
      p++,b++)
 {
  if (*p==QUOTE && !single)  quotes++;
  else if (quotes&1)
     ;
  else if (b==txt1 && *p==SINGLE)
      single=1;
  else if (single && *p==SINGLE)
      single=0;
  else if (*p=='(')  parent++;
  else if (*p==')')  parent--;
  if (parent < 0)  return MOP;
  *b=*p;
 }
 *b=0;
 if (quotes&1)  return UNQ; 
 if (single)  return UNS;
 if (parent > 0)  return MCP; 
 a->next=(struct argument_type *)malloc(sizeof(struct argument_type));
 if (!a->next){free(a);return IEM;}
 a->next->is_label=0;
 a->next->value=0;
 a->next->type=A_EMPTY;
 a->next->next=NULL;
 a->next->previous=(a==c)?NULL:a;
 a=a->next;
 if ((ret=convert_arg(a,txt1))) {free (c); return ret;}
 if (a->is_label && pruchod==1)
    if (lex.instruction == I_ORG  ||  lex.instruction == I_DEFS  ||
        lex.instruction == I_COND  ||  lex.instruction == I_ALIGN)
    {  if (!a->is_resolved)
          return EP1;
    }
    else if (lex.instruction == I_EQU  ||  lex.instruction == I_DEFL)
    {  if (!a->is_resolved)
          if (!(a->text=resolve_current_pc_and_store(txt1)))
             return IEM;
    }
    else
/**       if (a->type == A_PODLE_IY_PLUS || a->type == A_PODLE_IX_PLUS  ||
              c_adc_sbc || c_add || c_logical || c_in || c_out || c_ld)  **/
       a->value= 0;   /* avoids 8 bit overflows during pass 1 */
 while((*p)==TAB||(*p)==SPACE||(*p)==',')
  p++;
 if (!(*p)||(*p)==';'){lex.arg=c->next;free(c);return 0;}
 }
}


void
out(unsigned char a)
{
if (pruchod==1){address++;return;}

  if (LISTING && pruchod==2) printf(" %02x",(unsigned)a);
  if (write_to_memory(address,a) != fill_byte && WARNINGS) 
   fprintf(stderr,"Warning: overwriting code at address 0x%04x (%05u)\n",
           (unsigned)address,(unsigned)address);

if (!count || address>end)  end=address;
address++;
count++;
}


unsigned  compile_pass(void)
{
   return  pruchod;
}

void set_compile_pass(unsigned pass)
{
   pruchod=pass;
   lineno=(pass?1:0);
   last_true_cond_level= cond_level=0;
}

int generated_bytes(void)
{
   return  count;
}

unsigned short highest_address(void)
{
   return  end;
}

unsigned short get_current_address(void)
{
   return address;
}

void set_start_address(unsigned short addr)
{
   address=addr;
}

/*** see instr_token for the correct order ***/
static unsigned char  no_para[] =
     {  0 ,
        1<<2, 1<<0, 1<<0, 1<<0, 1<<0,
        1<<2, 1<<0, 1<<1, 1<<1,
        1<<1, 1<<1,
        1<<2, 1<<2, 1<<2,
        1<<1, 1<<1, 1<<1, 1<<1, 1<<1,
        1<<0, 1<<0, 1<<0, 1<<0,
        1<<1|1<<2, 1<<1, 1<<1|1<<2,
        1<<1|1<<2, 1<<1, 1<<0|1<<1, 1<<0, 1<<0,
        1<<1, 1<<0, 1<<1, 1<<0, 1<<1, 1<<0, 1<<1, 1<<0,
        1<<1, 1<<1, 1<<1, 1<<1,
        1<<0, 1<<0, 1<<0,
        1<<2, 1<<2, 1<<2,
        1<<0, 1<<0, 1<<0, 1<<0,
        1<<2, 1<<0, 1<<0, 1<<0, 1<<0,
        1<<2, 1<<0, 1<<0, 1<<0, 1<<0,
        1<<0, 1<<0, 1<<0, 1<<0, 1<<1,

        1<<0, 1<<1, 1<<1, 1<<1, ~0^1, ~0^1, 1<<1,
        1<<1, 1<<1,
        1<<1, 1<<0,
     } ;

/*** see asm_token for the correct order ***/
static char *msg[] = { "",
     /* IEM */         "Can't allocate sufficient memory.",
     /* LBNO */        "Label not defined.",
     /* LBAR */        "Label already declared.",
     /* ILB  */        "Invalid Labelname.",
     /* ILD  */        "Illegal Labeldeclaration.",
     /* ILO  */        "Illegal Operator.",
     /* UNQ  */        "Unbalanced Quotes.",
     /* MOP  */        "Missing Opening Parenthesis.",
     /* MCP  */        "Missing Closing Parenthesis.",
     /* UNS  */        "Unbalanced Single Quote.",
     /* STRT */        "String appended by text (missing quotes?).",
     /* PART */        "Parenthesis appended by text (missing parentheses?).",
     /* IAC  */        "Invalid character argument.",
     /* SYNT */        "Syntax error in expression.",
     /* DEX  */        "DEFL statement expected.",
     /* EP1  */        "Expression can't be evaluated in Pass 1.",
     /* MISL */        "Missing Label.",
     /* MISO */        "Missing Operand.",
     /* FOR  */        "Forbidden instruction.",
     /* ILL  */        "Illegal instruction.",
     /* UNK  */        "Unknown instruction.",
     /* MIS1 */        "Missing argument.",
     /* MIS  */        "Missing arguments.",
     /* MIS2 */        "Missing second argument.",
     /* EXT  */        "Extra argument.",
     /* TOO  */        "Too many arguments.",
     /* VOR  */        "Unsigned Value out of range.",
     /* OOR  */        "Signed Offset out of range.",
     /* IA   */        "Invalid argument.",
     /* IA1  */        "Invalid first argument.",
     /* IA2  */        "Invalid second argument.",
     /* IAT  */        "Invalid argument type.",
     /* IAS  */        "Invalid argument. String expected.",
     /* IAN  */        "Invalid argument. Number expected.",
     /* IAH  */        "Invalid argument. One Hexadigit expected.",
     /* IAV  */        "Invalid argument value.",
     /* TCC  */        "ENDC statement without COND statement.",
     /* MEC  */        "missing ENDC statement(s) at end of source."
                     } ;


unsigned
check_cond_nesting(void)
{
   if (cond_level)
      error(lineno,"",msg[MEC]);
   return  cond_level ? MEC : 0;
}


unsigned
compile(const char *txt)
/* return value: 0=OK, 1-18=errors */
{
int a;
struct argument_type *t;
int ret=0;
a=lexical_analysis(txt);
if (a >= 1 && a <= 18)
{
 error(lineno,txt,msg[a]);
 ret=1;
 return ret;
}
else if (a)
{
fprintf(stderr,"%d ",a);
 error(lineno,txt,"internal error");
 ret=1;
 return ret;
}

if (!ret)
{  if (LISTING && pruchod==2)
      printf("%04x",last=address);
   if (cond_level > last_true_cond_level)
   {  if (lex.instruction != I_ENDC && lex.instruction != I_COND)
         lex.instruction=I_EMPTY;   /* we ignore also I_ILLEGAL opcodes */
   }
   if (!lex.arg)  a=0;
   else if (!lex.arg->next) a=1;
   else if (!lex.arg->next->next)  a=2;
   else a=3;
   if (lex.instruction >= I_END && lex.instruction < N_INSTRUCTIONS  &&
       disable_pseudo)
   {  error(lineno,txt,msg[FOR]);ret=1; }
   else if (lex.instruction >= 0 && lex.instruction < N_INSTRUCTIONS &&
            no_para[lex.instruction] && !(no_para[lex.instruction]&1<<a))
   {  switch (a)
      {  case 0:  if(no_para[lex.instruction]&2)
                     a=MIS1;
                  else
                     a=MIS;
                  break;
         case 1:  if(no_para[lex.instruction]&1)
                     a=EXT;
                  else if(no_para[lex.instruction]&4)
                     a=MIS2;
                  else
                     a=MIS;
                  break;
         default: if (no_para[lex.instruction]&1)
                     a=EXT;
                  else if(!(no_para[lex.instruction]>>a))
                     a=TOO;
                  else
                     a=MIS;
                  break;
      }
      if (a) error(lineno,txt,msg[a]);ret=1;
   }
}
if (!ret)
{  a=0;
   switch (lex.instruction)
   {
      case I_END:
      if (disable_pseudo){error(lineno,txt,msg[FOR]);ret=1;break;}
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      ret=8;
      break;

      case I_EQU:
      if (disable_pseudo){error(lineno,txt,msg[FOR]);ret=1;break;}
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->type!=A_NUM){error(lineno,txt,msg[IAT]);ret=1;break;}
      if (lex.arg->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      if (!label_access){error(lineno,txt,msg[MISL]);ret=1;break;}
      update_last_added_entry(lex.arg->value,lex.arg->text,1);
      break;

      case I_DEFL:
      if (disable_pseudo){error(lineno,txt,msg[FOR]);ret=1;break;}
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->type!=A_NUM){error(lineno,txt,msg[IAT]);ret=1;break;}
      if (lex.arg->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      if (!label_access){error(lineno,txt,msg[MISL]);ret=1;break;}
      update_last_added_entry(lex.arg->value,lex.arg->text,0);
      break;

      case I_ORG:
      if (disable_pseudo){error(lineno,txt,msg[FOR]);ret=1;break;}
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->type!=A_NUM){error(lineno,txt,msg[IAN]);ret=1;break;}
      if (lex.arg->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      address=lex.arg->value;
      last=address; /* we write no fill bytes into listing */
      break;

      case I_ILLEGAL:
      error(lineno,txt,msg[ILL]);
      ret=1;
      break;

      case I_EMPTY:
      break;

      case I_HALT:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else out(0x76);
      break;

      case I_CCF:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else out(0x3f);
      break;
 
      case I_CPD:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else {out(0xed);out(0xa9);}
      break;
 
      case I_CPDR:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else {out(0xed);out(0xb9);}
      break;
 
      case I_CPI:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else {out(0xed);out(0xa1);}
      break;
 
      case I_CPIR:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else {out(0xed);out(0xb1);}
      break;
 
      case I_CPL:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else out(0x2f);
      break;
 
      case I_DAA:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else out(0x27);
      break;
 
      case I_DI:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else out(0xf3);
      break;
 
      case I_EI:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else out(0xfb);
      break;
 
      case I_EXX:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else out(0xd9);
      break;
 
      case I_IND:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else {out(0xed);out(0xaa);}
      break;
 
      case I_INI:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else {out(0xed);out(0xa2);}
      break;
 
      case I_INDR:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else {out(0xed);out(0xba);}
      break;
 
      case I_INIR:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else {out(0xed);out(0xb2);}
      break;
 
      case I_LDD:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else {out(0xed);out(0xa8);}
      break;
 
      case I_LDI:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else {out(0xed);out(0xa0);}
      break;
 
      case I_LDDR:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else {out(0xed);out(0xb8);}
      break;
 
      case I_LDIR:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else {out(0xed);out(0xb0);}
      break;
 
      case I_NEG:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else {out(0xed);out(0x44);}
      break;
 
      case I_NOP:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else out(0x00);
      break;
 
      case I_OTDR:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else {out(0xed);out(0xbb);}
      break;
 
      case I_OTIR:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else {out(0xed);out(0xb3);}
      break;
 
      case I_OUTD:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else {out(0xed);out(0xab);}
      break;
 
      case I_OUTI:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else {out(0xed);out(0xa3);}
      break;
 
      case I_RETI:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else {out(0xed);out(0x4d);}
      break;
 
      case I_RETN:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else {out(0xed);out(0x45);}
      break;
 
      case I_RLA:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else out(0x17);
      break;
 
      case I_RLCA:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else out(0x07);
      break;
 
      case I_RLD:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else {out(0xed);out(0x6f);}
      break;
 
      case I_RRA:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else out(0x1f);
      break;
 
      case I_RRCA:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else out(0x0f);
      break;
 
      case I_RRD:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else {out(0xed);out(0x67);}
      break;
 
      case I_SCF:
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      else out(0x37);
      break;

      case I_ADC:
      if (!lex.arg){error(lineno,txt,msg[MIS]);ret=1;break;}
      if ((lex.arg->type!=A_REG)||(lex.arg->value!=R_A&&lex.arg->value!=R_HL))
       {error(lineno,txt,msg[IA1]);ret=1;break;}
      if (!lex.arg->next)
       {error(lineno,txt,msg[MIS2]);ret=1;break;}
      if (lex.arg->next->next)
       {error(lineno,txt,msg[TOO]);ret=1;break;}
      a=c_adc_sbc(lex.arg->value,lex.arg->next->type,lex.arg->next->value,0x88,0x0a);
      if (a==1) {error(lineno,txt,msg[IA]);ret=1;break;}
      break;

      case I_ADD:
      if (!lex.arg){error(lineno,txt,msg[MIS]);ret=1;break;}
      if ((lex.arg->type!=A_REG)||(lex.arg->value!=R_A&&lex.arg->value!=R_HL&&lex.arg->value!=R_IX&&lex.arg->value!=R_IY))
       {error(lineno,txt,msg[IA1]);ret=1;break;}
      if (!lex.arg->next)
       {error(lineno,txt,msg[MIS2]);ret=1;break;}
      if (lex.arg->next->next)
       {error(lineno,txt,msg[TOO]);ret=1;break;}
      a=c_add(lex.arg->value,lex.arg->next->type,lex.arg->next->value);
      if (a==1) {error(lineno,txt,msg[IA]);ret=1;break;}
      break;

      case I_AND:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      a=c_logical(lex.arg->type,lex.arg->value,0xa0);
      if (a==1) {error(lineno,txt,msg[IA]);ret=1;break;}
      break;

      case I_BIT:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->type!=A_NUM||(lex.arg->type==A_NUM&&lex.arg->value>7))
       {error(lineno,txt,msg[IA1]);ret=1;break;}
      if (!lex.arg->next){error(lineno,txt,msg[MIS2]);ret=1;break;}
      if (lex.arg->next->next)
       {error(lineno,txt,msg[TOO]);ret=1;break;}
      a=c_bit(lex.arg->value,lex.arg->next->type,lex.arg->next->value,0x40);
      if (a==1){error(lineno,txt,msg[IA2]);ret=1;break;}
      break;

      case I_CP:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      a=c_logical(lex.arg->type,lex.arg->value,0xb8);
      if (a==1) {error(lineno,txt,msg[IA]);ret=1;break;}
      break;

      case I_DEC:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      a=c_inc_dec(lex.arg->type,lex.arg->value,0x05,0x0b);
      if (a==1){error(lineno,txt,msg[IA]);ret=1;break;}
      break;
 
      case I_EX:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (!lex.arg->next){error(lineno,txt,msg[MIS2]);ret=1;break;}
      if (lex.arg->next->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      if (lex.arg->next->type!=A_REG){error(lineno,txt,msg[IA2]);ret=1;break;}
      if (c_ex(lex.arg->type,lex.arg->value,lex.arg->next->value))
       {error(lineno,txt,msg[IA]);ret=1;}
      break;

      case I_IM:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      if (lex.arg->type!=A_NUM||(lex.arg->type==A_NUM&&lex.arg->value>2))
       {error(lineno,txt,msg[IA]);ret=1;break;}
      c_im(lex.arg->value);
      break;
 
      case I_IN:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (!lex.arg->next){error(lineno,txt,msg[MIS2]);ret=1;break;}
      if (lex.arg->next->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      if (lex.arg->type!=A_REG){error(lineno,txt,msg[IA1]);ret=1;break;}
      a=c_in(lex.arg->value,lex.arg->next->type,lex.arg->next->value);
      if (a==1) {error(lineno,txt,msg[IA]);ret=1;break;}
      break;

      case I_INC:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      a=c_inc_dec(lex.arg->type,lex.arg->value,0x04,0x03);
      if (a==1){error(lineno,txt,msg[IA]);ret=1;break;}
      break;
 
      case I_OR:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      a=c_logical(lex.arg->type,lex.arg->value,0xb0);
      if (a==1) {error(lineno,txt,msg[IA]);ret=1;break;}
      break;

      case I_OUT:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (!lex.arg->next){error(lineno,txt,msg[MIS2]);ret=1;break;}
      if (lex.arg->next->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      if (lex.arg->next->type!=A_REG){error(lineno,txt,msg[IA2]);ret=1;break;}
      a=c_out(lex.arg->type,lex.arg->value,lex.arg->next->value);
      if (a==1) {error(lineno,txt,msg[IA]);ret=1;break;}
      break;

      case I_POP:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      if (lex.arg->type!=A_REG){error(lineno,txt,msg[IA]);ret=1;break;}
      if (c_push_pop(lex.arg->value,0x01))
       {error(lineno,txt,msg[IA]);ret=1;}
      break;
 
      case I_PUSH:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      if (lex.arg->type!=A_REG){error(lineno,txt,msg[IA]);ret=1;break;}
      if (c_push_pop(lex.arg->value,0x05))
       {error(lineno,txt,msg[IA]);ret=1;}
      break;
 
      case I_RES:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->type!=A_NUM||(lex.arg->type==A_NUM&&lex.arg->value>7))
       {error(lineno,txt,msg[IA1]);ret=1;break;}
      if (!lex.arg->next){error(lineno,txt,msg[MIS2]);ret=1;break;}
      if (lex.arg->next->next)
       {error(lineno,txt,msg[TOO]);ret=1;break;}
      a=c_bit(lex.arg->value,lex.arg->next->type,lex.arg->next->value,0x80);
      if (a==1) {error(lineno,txt,msg[IA2]);ret=1;break;}
      break;

      case I_RET:
      if (!lex.arg){out(0xc9);break;}
      if (lex.arg->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      if (lex.arg->type==A_REG)
      {
       if (lex.arg->value!=R_C){error(lineno,txt,msg[IA]);ret=1;break;}
      }
      else {if (lex.arg->type!=A_FLAG){error(lineno,txt,msg[IA]);ret=1;break;}}
      if (c_ret(lex.arg->value))
       {error(lineno,txt,msg[IA]);ret=1;}
      break;
 
      case I_RL:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      a=c_shift_rot(lex.arg->type,lex.arg->value,0x10);
      if (a==1){error(lineno,txt,msg[IA]);ret=1;break;}
      break;

      case I_RLC:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      a=c_shift_rot(lex.arg->type,lex.arg->value,0x00);
      if (a==1){error(lineno,txt,msg[IA]);ret=1;break;}
      break;

      case I_RR:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      a=c_shift_rot(lex.arg->type,lex.arg->value,0x18);
      if (a==1){error(lineno,txt,msg[IA]);ret=1;break;}
      break;

      case I_RRC:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      a=c_shift_rot(lex.arg->type,lex.arg->value,0x08);
      if (a==1){error(lineno,txt,msg[IA]);ret=1;break;}
      break;

      case I_RST:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      if (lex.arg->type!=A_NUM){error(lineno,txt,msg[IAT]);ret=1;break;}
      if (c_rst(lex.arg->value))
       {error(lineno,txt,msg[IAV]);ret=1;}
      break;

      case I_SBC:
      if (!lex.arg){error(lineno,txt,msg[MIS]);ret=1;break;}
      if ((lex.arg->type!=A_REG)||(lex.arg->value!=R_A&&lex.arg->value!=R_HL))
       {error(lineno,txt,msg[IA1]);ret=1;break;}
      if (!lex.arg->next)
       {error(lineno,txt,msg[MIS2]);ret=1;break;}
      if (lex.arg->next->next)
       {error(lineno,txt,msg[TOO]);ret=1;break;}
      a=c_adc_sbc(lex.arg->value,lex.arg->next->type,lex.arg->next->value,0x98,0x02);
      if (a==1) {error(lineno,txt,msg[IA]);ret=1;break;}
      break;

      case I_SET:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->type!=A_NUM||(lex.arg->type==A_NUM&&lex.arg->value>7))
       {error(lineno,txt,msg[IA1]);ret=1;break;}
      if (!lex.arg->next){error(lineno,txt,msg[MIS2]);ret=1;break;}
      if (lex.arg->next->next)
       {error(lineno,txt,msg[TOO]);ret=1;break;}
      a=c_bit(lex.arg->value,lex.arg->next->type,lex.arg->next->value,0xc0);
      if (a==1) {error(lineno,txt,msg[IA2]);ret=1;}
      break;

      case I_SLA:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      a=c_shift_rot(lex.arg->type,lex.arg->value,0x20);
      if (a==1){error(lineno,txt,msg[IA]);ret=1;break;}
      break;

      case I_SLL:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      a=c_shift_rot(lex.arg->type,lex.arg->value,0x30);
      if (a==1){error(lineno,txt,msg[IA]);ret=1;break;}
      break;

      case I_SRA:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      a=c_shift_rot(lex.arg->type,lex.arg->value,0x28);
      if (a==1){error(lineno,txt,msg[IA]);ret=1;break;}
      break;

      case I_SRL:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      a=c_shift_rot(lex.arg->type,lex.arg->value,0x38);
      if (a==1){error(lineno,txt,msg[IA]);ret=1;break;}
      break;

      case I_SUB:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      a=c_logical(lex.arg->type,lex.arg->value,0x90);
      if (a==1) {error(lineno,txt,msg[IA]);ret=1;break;}
      break;

      case I_XOR:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      a=c_logical(lex.arg->type,lex.arg->value,0xa8);
      if (a==1) {error(lineno,txt,msg[IA]);ret=1;break;}
      break;

      case I_JP:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (!lex.arg->next)
       {
       if (c_jp(lex.arg->type,lex.arg->value,A_EMPTY,0))
        {error(lineno,txt,msg[IA]);ret=1;break;}
       }
      else 
       {
       if (lex.arg->next->next){error(lineno,txt,msg[TOO]);ret=1;break;}
       if (c_jp(lex.arg->type,lex.arg->value,lex.arg->next->type,lex.arg->next->value))
        {error(lineno,txt,msg[IA]);ret=1;break;}
       }
      break;
 
      case I_CALL:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (!lex.arg->next)
       {
       if (c_call(lex.arg->type,lex.arg->value,A_EMPTY,0))
        {error(lineno,txt,msg[IA]);ret=1;break;}
       }
      else 
       {
       if (lex.arg->next->next){error(lineno,txt,msg[TOO]);ret=1;break;}
       if (c_call(lex.arg->type,lex.arg->value,lex.arg->next->type,lex.arg->next->value))
        {error(lineno,txt,msg[IA]);ret=1;break;}
       }
      break;
 
      case I_JR:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (!lex.arg->next)
       a=c_jr(lex.arg->type,lex.arg->value-(lex.arg->is_label?address+2:0),A_EMPTY,0);
      else 
       {
       if (lex.arg->next->next){error(lineno,txt,msg[TOO]);ret=1;break;}
       a=c_jr(lex.arg->type,lex.arg->value,lex.arg->next->type,
                lex.arg->next->value-(lex.arg->next->is_label?address+2:0));
       }
      if(a==1){error(lineno,txt,msg[IA]);ret=1;break;}
      break;
 
      case I_DJNZ:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      a=c_djnz(lex.arg->type,lex.arg->value-(lex.arg->is_label?address+2:0));
      if(a==1){error(lineno,txt,msg[IA]);ret=1;break;}
      break;
 
      case I_LD:
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (!lex.arg->next){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->next->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      a=c_ld(lex.arg->type,lex.arg->value,lex.arg->next->type,lex.arg->next->value);
      if (a==1){error(lineno,txt,msg[IA]);ret=1;break;}
      break;

      case I_ALIGN:
      if (disable_pseudo){error(lineno,txt,msg[FOR]);ret=1;break;}
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      t=lex.arg;
      if (t->type!=A_NUM){error(lineno,txt,msg[IAH]);ret=1;break;}
      if (t->value >15 || t->value<0){error(lineno,txt,msg[VOR]);ret=1;break;}
      if (address&(1<<t->value)-1) address= ((address>>t->value)+1)<<t->value;
      last=address; /* we write no fill bytes into listing */
      break;

      case I_DEFS:
      if (disable_pseudo){error(lineno,txt,msg[FOR]);ret=1;break;}
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      t=lex.arg;
      if (t->type!=A_NUM){error(lineno,txt,msg[IAN]);ret=1;break;}
      if (t->value>65535||t->value<0){error(lineno,txt,msg[VOR]);ret=1;break;}
      if (address+t->value>=65536)  address=0; else address += t->value;
      last=address; /* we write no fill bytes into listing */
      break;

      case I_DEFM:
      if (disable_pseudo){error(lineno,txt,msg[FOR]);ret=1;break;}
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      t=lex.arg;
      do
      {
       unsigned a;
       bool saved_listing_mode=LISTING;
       if (t->type!=A_STRING){error(lineno,txt,msg[IAS]);ret=1;break;}
       for (a=0;a<strlen(t->text);a++)
       {  if (a>=4)  LISTING=0;
          out(t->text[a]);
       }
       LISTING=saved_listing_mode;
       free(t->text);
       t=t->next;
      }
      while (t);
      break;

      case I_DEFB:
      if (disable_pseudo){error(lineno,txt,msg[FOR]);ret=1;break;}
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      t=lex.arg;
      do
      {
      if (t->type!=A_NUM){error(lineno,txt,msg[IAN]);ret=1;break;}
      if (t->value>255||t->value<-128){error(lineno,txt,msg[VOR]);ret=1;break;}
      out(t->value);
      t=t->next;
      }
      while (t);
      break;
 
      case I_DEFW:
      if (disable_pseudo){error(lineno,txt,msg[FOR]);ret=1;break;}
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      t=lex.arg;
      do
      {
      if (t->type!=A_NUM){error(lineno,txt,msg[IAN]);ret=1;break;}
      if (t->value>65535||t->value<-32768){error(lineno,txt,msg[VOR]);ret=1;break;}
      out(t->value&255);out((t->value>>8)&255);
      t=t->next;
      }
      while (t);
      break;
 
      case I_COND:
      if (disable_pseudo){error(lineno,txt,msg[FOR]);ret=1;break;}
      if (!lex.arg){error(lineno,txt,msg[MIS1]);ret=1;break;}
      if (lex.arg->type!=A_NUM){error(lineno,txt,msg[IAT]);ret=1;break;}
      if (lex.arg->next){error(lineno,txt,msg[TOO]);ret=1;break;}
      if (cond_level == last_true_cond_level && lex.arg->value)
         last_true_cond_level++;
      cond_level++;
      break;

      case I_ENDC:
      if (disable_pseudo){error(lineno,txt,msg[FOR]);ret=1;break;}
      if (lex.arg){error(lineno,txt,msg[EXT]);ret=1;}
      if (!cond_level){error(lineno,txt,msg[TCC]);ret=1;break;}
      if (last_true_cond_level == cond_level)
         last_true_cond_level--;
      cond_level--;
      break;

      default:
      error(lineno,txt,msg[UNK]);
      ret=1;
      break;
   }
   if (pruchod!=1 && a==8) {error(lineno,txt,msg[VOR]);ret=1;}
   if (pruchod!=1 && a==9) {error(lineno,txt,msg[OOR]);ret=1;}

   if (!(ret&7) && LISTING && pruchod==2)
   { static char blank[13];
     int i;
     for (i=0;i<12;i++) blank[i]=SPACE;
     i= address-last;
     printf("%s%5u %s\n",blank+3*(i>=1&&i<=4?i:(i>4?4:0)),lineno,txt);
   }
}
lineno++;
if (lex.arg)
 {
 t=lex.arg;
 while((t=t->next))
  free(t->previous);
 free(t);
 }

return ret;
}
