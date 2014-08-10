#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "asm.h"
#include "hash.h"
#include "asm_token"

#define   SINGLE '\''

#define   HEX_PREFIX '$'
#define   BIN_PREFIX '#'
#define   CURRENT_PC '@'


int
test_label(char *txt)
{
int i;

for (i=0;i<MAX_LABEL;i++)
   if (txt[i] >= 'A' && txt[i] <= 'Z')
       ;
   else if (txt[i] >= 'a' && txt[i] <= 'z')
       ;
   else if (i && txt[i] >= '0' && txt[i] <= '9')
       ;
   else if (txt[i] != '_')
      return i;
return 0;
}


/* a number must start with either '\'', '+', '-', digit or BIN_/HEX_PREFIX */
int
test_number(char *txt,int *value)
{
int base;
char *p,*q;
int sign=1, a, b;
p=txt;
/* character constant */
if (txt[0]==SINGLE && txt[1] && txt[2]==SINGLE)
{
 *value=(unsigned char)txt[1];
 return 3;
}
if (*p=='-')
{ sign= -1; p++; }
else if (*p=='+')
  p++;   /* we like it for JR, DJNZ argument */
base=10;
if (*p==BIN_PREFIX)
{ base=2; p++; }
else if (*p==HEX_PREFIX)
{ base=16; p++; }
else if (*p=='0' && (p[1]=='x'||p[1]=='X'))
{ base=16; p+=2; }
a=b=0;
for (q=p ; *p ; p++)
{
   if (*p >= '0' && *p <= '1')
      ;
   else if (*p >= '0' && *p <= '9')
      a=1;
   else if (!a && !b && base==10 && (*p == 'b' || *p == 'B'))
   {  if(p==q) return 0;
      base=3;
   }
   else if ((*p >='a' && *p <= 'f') || (*p >='A' && *p <= 'F'))
   {  if (p==q && base!=16) return 0;
      b=1;
   }
   else if (*p == 'h' || *p == 'H')
   {  if(p==q || (base!=10 && base !=3)) return 0;
      p++; base=16;
      break;
   }
   else 
      break;
}
if (base==3 && *(p-1) != 'b' && *(p-1) != 'B')
   return 0;
else if (base==3)
   base=2;
if (b && base != 16 || p==q)
   return 0;
*value=sign*strtol(q,NULL,base);
return  p-txt;
}


char *
resolve_current_pc_and_store(char *txt)
{  int a, b;
   char  *p, *q;
   for (a=b=0;txt[b];b++)
      a +=  txt[b]==CURRENT_PC;
   if (q=malloc(strlen(txt)+1+4*a))
      for (a=0,p=q;*p=txt[a];p++,a++)
         if (*p==CURRENT_PC)
         {  sprintf(p,"%c%.4x",HEX_PREFIX,get_current_address());
            p += 4;
         }
   return  q;
}


#define  STACK_SIZE  16
#define  LIST_SIZE  256

static unsigned  stack_top, list_top;
static struct info {
       unsigned char type;
       int value;
} *stack, *list;


static void swap_internal_data( struct info **new_stack, struct info **new_list,
                                unsigned *new_stack_top, unsigned *new_list_top)
{
unsigned  temp;
struct info *scra;
temp= stack_top;  stack_top= *new_stack_top; *new_stack_top=temp;
temp=list_top;  list_top= *new_list_top; *new_list_top=temp;
scra= *new_stack;  *new_stack=stack;  stack=scra;
scra= *new_list;  *new_list=list;  list=scra;
}


static void push_on_stack(unsigned typ, int val)
{
   static struct info t;
   t.type= typ;
   t.value= val;
   if (stack_top < STACK_SIZE)
      stack[stack_top++] = t;
}


static void append_to(unsigned typ, int val)
{
   static struct info t;
   t.type= typ;
   t.value= val;
   if (list_top < LIST_SIZE)
      list[list_top++] = t;
}


static void append(struct info t)
{
   if (list_top < LIST_SIZE)
      list[list_top++] = t;
}


static void skim_higher_priorities(char *lower_priorities, char binary_operator)
{
   while (stack_top)
   {  struct info  token=stack[stack_top-1];
      if (strchr(lower_priorities,token.type))
         break;
      else
         append(stack[--stack_top]);
   }
   push_on_stack(binary_operator,2);
}


/* return value:
   < 0 error and this is the error code/reason
          lexical_analysis(txt) will return |value|  used in compile(txt)
   = 0 everything ok and defined
   > 0 expression ok, but value undefined (return type of expression?)
*/
/* monadic operators and ( as well, are always pushed onto stack */
/* if binary operator appears, then first stack is poped until an operator
   of low priority or equal priority and right associativity appears.
   (so, all higher priority or equal priority and left associative operators
    are evaluated first)
   if ) then stack is poped including next ( appearance
*/
/*  Operators for stack:

30  (     3    opening pharenthesis 
28  ~     1    monadic bitwise 1-complement
28        1    monadic plus sign
28  _     1    monadic minus sign (bitwise 2-complement)
28  // k  1    monadic bitsize (log_2 styricly rounded up)
28  ?     6    monadic is_label_defined
26  ** l  2    binary power
24  %     2    binary modulo
24  /     2    binary division
24  *     2    binary multiplication
20  +     2    binary addition
20  -     2    binary subtraction
19  >> a  2    binary bitwise right shift  /2^
19  << b  2    binary bitwise left shift   *2^
18  &     2    binary bitwise-and
16  |     2    binary bitwise-or
16  ^     2    binary bitwise-xor
14  !     1    monadic boolean == 0 
12  >  c  2    binary boolean >
12  <  d  2    binary boolean <
12  == e  2    binary boolean =
12  != f  2    binary boolean !=
12  >= g  2    binary boolean >=
12  <= h  2    binary boolean <=
 8  && i  2    binary boolean and
 6  || j  2    binary boolean or
 2  )     4    closing pharenthesis 
 0        3    nothing (start initialization)
    @     5    current PC
  number  5    numbers or labels
*/    
int
parse_expr(char *txt, int* value, unsigned lineno)
{
   struct info  token, my_stack[STACK_SIZE], my_list[LIST_SIZE];
   struct info  *my_stack_ptr=my_stack, *my_list_ptr=my_list;
   unsigned  a,t, b=3, undef=0, pruchod=compile_pass();
   unsigned  my_list_top=0, my_stack_top= 0;
   int  ret=0;

   if (!txt)  return  -SYNT;
   swap_internal_data(&my_stack_ptr, &my_list_ptr, &my_stack_top, &my_list_top);

   for (a=0;txt[a];a+=t)
   {  t=1;
      switch (txt[a])
      {
         case '>': 
                    if (b != 4 && b != 5)
                    { ret=-MISO;  goto finish; }
                    if (txt[a+1]=='=') /* comparision operator >= */
                    {  skim_higher_priorities("(ij",'g');
                       t=2;
                    }
                    else if (txt[a+1]=='>') /* shift operator >> */
                    {  skim_higher_priorities("(ijghefcd^|&",'a');
                       t=2;
                    }
                    else /* comparision operator > */
                    {  skim_higher_priorities("(ij",'c');
                    }
                    b=2;
                    break;
         case '<':
                    if (b != 4 && b != 5)
                    { ret=-MISO;  goto finish; }
                    if (txt[a+1]=='=') /* comparision operator <= */
                    {  skim_higher_priorities("(ij",'h');
                       t=2;
                    }
                    else if (txt[a+1]=='<') /* shift operator << */
                    {  skim_higher_priorities("(ijghefcd^|&",'b');
                       t=2;
                    }
                    else /* comparision operator < */
                    {  skim_higher_priorities("(ij",'d');
                    }
                    b=2;
                    break;
         case '=':
                    if (b != 4 && b != 5)
                    { ret=-MISO;  goto finish; }
                    if (txt[a+1]=='=')   /* comparision operator == */
                    {  skim_higher_priorities("(ij",'e');
                       b=2;  t=2;
                       break;
                    }
                    else
                    { ret=-ILO;  goto finish; }
         case '!':
                    if (txt[a+1]=='=' ? b!=4 && b!=5 : b!=3 && b!=4 && b!=5)
                    { ret=-MISO;  goto finish; }
                    if (txt[a+1]=='=')   /* comparision operator != */
                    {  skim_higher_priorities("(ij",'f');
                       b=2;  t=2;
                    }
                    else  /* boolean not */
                    {
                       push_on_stack('!',1);
                       b=1;
                    }
                    break;
         case '~':
                    if (b != 3 && b !=4 && b != 5)
                    { ret=-MISO;  goto finish; }
                    push_on_stack(txt[a],1);
                    b=1;
                    break;
         case '&':
         case '|':
         case '^':
                    if (b !=4 && b != 5)
                    { ret=-MISO;  goto finish; }
                    if (txt[a]=='&' && txt[a+1]=='&') /* boolean and */
                    {  skim_higher_priorities("(j",'i');
                       t=2;
                    }
                    if (txt[a]=='|' && txt[a+1]=='|') /* boolean or */
                    {  skim_higher_priorities("(",'j');
                       t=2;
                    }
                    if (txt[a]=='&') /* bitwise and operator */
                    {  skim_higher_priorities("(jihgfedc|^",txt[a]);
                    }
                    else /* bitwise or/xor operator */
                    {  skim_higher_priorities("(jihgfedc",txt[a]);
                    }
                    b=2;
                    break;
         case '+':
         case '-':
                    if (b==4 || b == 5)
                       skim_higher_priorities("(jihgfedc|^&bc",txt[a]);
                    else
                       push_on_stack((txt[a]=='-'?'_':' '),1);
                    b=(b<=3?1:2);
                    break;
         case '%':
         case '/':
                    if (txt[a+1]=='/' ? b!=3 : b!=4 && b!=5)
                    { ret=-MISO;  goto finish; }
                    if (txt[a+1]=='/')   /* monadic bitsize */
                    {  push_on_stack('k',1);
                       b=1;  t=2;
                    }
                    else
                    {  skim_higher_priorities("(jihgfedc|^&bc+-",'/');
                       b=2;
                    }
                    break;
         case '*':  if (b != 4 && b != 5)
                    { ret=-MISO;  goto finish; }
                    if (txt[a+1]=='*')   /* binary power */
                    {  skim_higher_priorities("(jihgfedc|^&bc+-%/*",'l');
                       b=2;  t=2;
                    }
                    else
                    {  skim_higher_priorities("(jihgfedc|^&bc+-",'*');
                       b=2;
                    }
                    break;
         case '(':
                    if (b != 1 && b != 2 && b != 3)
                    { ret=-PART;  goto finish; }
                    push_on_stack('(',0);
                    b=3;
                    break;
         case ')':
                    if (b != 4 && b != 5)
                    { ret=-MISO;  goto finish; }
                    while (stack_top)
                    {  token=stack[--stack_top];
                       if (token.type=='(')  break;
                       else append(token);
                    }
                    if (token.type != '(')
                    { ret=-PART;  goto finish; }
                    b=4;
                    break;
         case CURRENT_PC:
                    if (b != 1 && b != 2 && b != 3)
                    { ret=-PART;  goto finish; }
                    append_to(255,get_current_address());
                    b=5;
                    break;
         case '?':  
                    if (b != 3 && b !=4 && b != 5)
                    { ret=-MISO;  goto finish; }
                    push_on_stack(txt[a],1);
                    b=6;
                    break;
         default:
       
                    if (b!=1 && b!=2 && b!=3 && b!=6)
                    { ret=-PART;  goto finish; }
                    if (b==6)
                    {  if ((t=test_label(txt+a)))
                       {  *value= is_in_table(txt+a,t,0,0);
                       }
                       else
                       { ret=-MISL;  goto finish; }
                    }
                    else if ((t=test_number(txt+a,value)))
                    {  append_to(255,*value);
                       b=5;
                    }
                    else if ((t=test_label(txt+a)))
                    {  if (!is_in_table(txt+a,t,value,lineno))
                       {  if (pruchod==1)
                             *value=get_current_address();
                          else
                          { ret=-LBNO;  goto finish; }
                          undef=1;
                       }
                       append_to(255,*value);
                       b=5;
                    }
                    else
                    { ret=-ILO;  goto finish; }
      }
   }
   if (!undef)
   {  int c;
      int left=0, right=0, resu=0;
      while (stack_top)
      {  token=stack[--stack_top];
         if (token.type=='(')
            { ret=-PART;  goto finish; }
         else
            append(token);
      }
      for (a=0;a<list_top;a++)
      {  if (list[a].type == 255)  continue;
         c= list[a].value;
         for (b=a;c && b--;)
            if (list[b].type == 255)
            {  if (c-- == 2)  right=list[b].value;
               else  left=list[b].value; 
               list[b].type= 0;
            }
         switch(list[a].type)
         {   case '?':   resu=  left;
                         break;
             case ' ':   resu=  left;
                         break;
             case '_':   resu=  -left;
                         break;
             case '+':   resu=  left+right;
                         break;
             case '-':   resu=  left-right;
                         break;
             case '*':   resu= left*right;
                         break;
             case '/':   resu= left/right;
                         if (right*resu > left)  resu--;
                         break;
             case '%':   resu= right ? left%right : left;
                         if (right > 0 && resu < 0)  resu += right;
                         break;
             case '~':   resu= ~left;
                         break;
             case '|':   resu= left|right;
                         break;
             case '^':   resu= left^right;
                         break;
             case '&':   resu= left&right;
                         break;
             case '!':   resu= -(!left);
                         break;
             case 'a':   if ((unsigned)right >= sizeof(left)*8)
                            resu= (left >= 0 ? 0 : -1);
                         else
                            resu= left>>right;
                         break;
             case 'b':   if ((unsigned)right >= sizeof(left)*8)
                            resu= (left >= 0 ? 0 : 0);
                         else
                            resu= left<<right;
                         break;
             case 'c':   resu= -(left>right);
                         break;
             case 'd':   resu= -(left<right);
                         break;
             case 'e':   resu= -(left==right);
                         break;
             case 'f':   resu= -(left!=right);
                         break;
             case 'g':   resu= -(left>=right);
                         break;
             case 'h':   resu= -(left<=right);
                         break;
             case 'i':   resu= -(left&&right);
                         break;
             case 'j':   resu= -(left||right);
                         break;
             case 'k':   {  unsigned v=left;
                            for (resu=0;v;v>>=1,resu++);
                         }
                         break;
             case 'l':   {  switch ((unsigned)right)
                            {  case 0:  resu=1;
                                        break;
                               case 1:  resu=left;
                                        break;
                               case 2:  resu= left*left;
                                        break;
                               default: if (!left)
                                           resu=0;
                                        else if (left==1)
                                           resu=1;
                                        else if (left== -1)
                                           resu= right&1 ? -1 : 1;
                                        else if (left==2)
                                           resu=1<<(unsigned)right;
                                        else if (left== -2)
                                           resu=(right&1?-1:1)<<(unsigned)right;
                                        else
                                        {  unsigned v=right;
                                           for (resu=left;--v;resu *= left);
                                        }
                            }  
                         }
                         break;
         }
         list[b].type= 255;
         list[b].value= resu;
      }
      *value=list[0].value;
   }
   else
      ret=undef;
   finish:
   swap_internal_data(&my_stack_ptr, &my_list_ptr, &my_stack_top, &my_list_top);
   return  ret;
}
