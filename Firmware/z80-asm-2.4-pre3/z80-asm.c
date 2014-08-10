#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "hash.h"
#include "asm.h"
#include "file.h"

_uchar F,A,B,C,D,E,H,L;
_uchar F_,A_,B_,C_,D_,E_,H_,L_;
_uchar IXl,IXh,IYl,IYh;
_uchar I,R, IM;
_ushort PC,SP,IX,IY;
_uchar DATA; /* data pins */
_ushort ADDRESS; /* address pins */
bit IFF0, IFF1,IFF2, IFF3;  /* internal EI-flipflop & interrupt flip flops */
enum cpu_control_pin { rd, wr, iorq, mreq, m1, inter, halt, wait, reset, rfsh,
                       busrq, busack };
bit cpu_pin[NO_CPU_CONTROL_PINS];

static FILE *input;

static unsigned char memory[65536];
static unsigned short start,length;


unsigned char
write_to_memory(unsigned short index, unsigned char a)
{
 unsigned char previous=memory[index];
 memory[index]=a;
 return previous;
}


static int
take_line(char *line, int max_chars)
{
 if (!fgets(line,max_chars-1,input)){line[0]=0;return 1;}  /* end of file */
#ifdef UNIX
 if (strlen(line) && line[strlen(line)-1]=='\n')
#endif
 line[strlen(line)-1]=0;
#ifdef UNIX  /* reading DOS-files */
 if (strlen(line) && line[strlen(line)-1]=='\r')
    line[strlen(line)-1]=0;
#endif
#ifdef DOS
 if (!strlen(line))take_line(line,max_chars);
#endif
 return 0;
}


static void
usage(char *myname)
{
printf(
"Z80 assembler.  "
"(c)1999-2004 Brainsoft  (Copyleft) 1999-2005\n"
"Usage: %s [-w] [-h] [-l] [-f xx] [-c] <input file .asm> [<start address>[[:<length>]:<output file>]]\n"
"Usage: %s [-w] [-h] [-l] [-f xx] [-c] <input file> [<start address>[:<length>]:<output file>] ...\n"
,myname,myname);
}


void
error(int l,char *line,char *txt)
{
fprintf(stderr,"%s\nline %d: %s\n",line,l,txt);
}


/* return value: 0=OK, 1=syntax error */
/* parses command line options */

static int
parse_arg(char *arg,char **filename)
{
   unsigned long a;
   char *p, *q, *e;
 
   q=arg;
   if (!*q && !*filename)
   {  fprintf(stderr,"Error: Output filename not specified.\n");
      return 1;
   }
   for (p=arg ; (*p)&&(*p)!=':' ; p++);  /* find ':' or end of string */
   if (*p)
   {  *p=0;
      p++;
      a=strtoul(arg,&e,16);
      if (*e)
      {  fprintf(stderr,"Error: Starting address is not a number.\n");
         return 1;
      }
      if (a >= 1<<16)
      {  fprintf(stderr,"Error: Starting address out of range.\n");
         return 1;
      }
      start=(unsigned short)a;
      q=p;
   }
   else
      start=0;
   for (; (*p)&&(*p)!=':' ; p++);  /* find ':' or end of string */
   if (*p) /* start:length:filename */
   {
      *p=0;
      p++;
      a=strtoul(q,&e,10); 
      if (*e)
      {  fprintf(stderr,"Error: Length is not a number.\n");
         return 1;
      }
      if (a>=1<<16)
      {  fprintf(stderr,"Error: Length out of range.\n");
         return 1;
      }
      length=(unsigned short)a;
      if ((int)start+length >= 1<<16)
      {  fprintf(stderr,"Error: File is too long.\n");
         return 1;
      }
      q=p;
   }
   else
      length=0;  /* start:filename */
   if (*q)
      *filename=q;
   return 0;
}


struct info{ char *label; int value; unsigned lineno; };

static int compare(const struct info *left, const struct info *right)
{
   return  strcmp(left->label,right->label);
}   

int
main(int argc, char **argv)
{
FILE *output;
char *txt=0;
char line[512];
int s,a=0,b,cross=0;
/* a is used as memory init value */

for (b=s=1;s<argc&&*argv[s]=='-';b++)
{
   if (!*(argv[s]+b))
      b=1, s++;
   else if (*(argv[s]+b)=='w')
   {  printf("Warnings switched on.\n");WARNINGS=1;
   }
   else if (*(argv[s]+b)=='l') LISTING=1;
   else if (*(argv[s]+b)=='c') cross=1;
   else if (*(argv[s]+b)=='f')
   {  if (s+1>=argc || 1!=sscanf(argv[++s],"%2x",&a))
         fprintf(stderr,"Error: option -f needs a hexadecimal argument\n");
      else
         b=0, s++;
   }
   else if (*(argv[s]+b)=='h') {usage(argv[0]);}
   else if (*(argv[s]+b)=='?') {usage(argv[0]);}
   else if (*(argv[s]+b)=='-') {s++;break;} /* in case filename equals option */
   else fprintf(stderr,"Error: unknown option -%s\n",argv[s]+b);
}

if (s == argc) return 0;
input=fopen(argv[s],"r");
if (!input){fprintf(stderr,"Error: can't open input file \"%s\".\n",argv[s]);return 1;}

disable_pseudo=0;
memset(memory,a,1<<16);
asm_init((unsigned char)a);

a=0;
set_compile_pass(1);
set_start_address(0);
while (!a && !take_line(line,511))
  a= compile(line);
if (a==8) a=0;
if (!a)
  a=check_cond_nesting();
if (!a)
 {
 fseek(input,0,SEEK_SET);
 set_compile_pass(2);
 set_start_address(0);
 while (!a && !take_line(line,511))
   a= compile(line);
 }
if (a==8) a=0;
fclose(input);
if (s+1 == argc)
{asm_close();fprintf(stderr,"No code generated\n");return 0;}
if (!a)
{
   for (b=1+s;b<argc;b++)
   {
      txt=0;
      if (argc==2+s && strlen(argv[s]) >= 5 &&
          !strcmp(argv[s]+strlen(argv[s])-4,".asm"))
      {  txt=argv[s];
         sprintf(txt+strlen(argv[s])-3,"z80");
      }
      if (parse_arg(argv[b],&txt))
      {  asm_close();  return 1;  }
      output=fopen(txt,"wb");
      if (!output)
      {  asm_close();
         fprintf(stderr,"Error: Can't open output file \"%s\".\n",txt);
         return 1;
      }
      write_header(output,start);
      if (length || generated_bytes() && highest_address() >= start)
         fwrite(memory+start,1,length?length:highest_address()+1-start,output);
      fclose(output);
   }
}
if (!a && (LISTING||cross))
   printf("%u bytes code generated and %u labels defined\n",
          generated_bytes(),table_entries());
if (!a && cross && (b=table_entries()))
{ struct info  *ele;
  ele= malloc(b*sizeof(struct info));
  for(a=0;next_table_entry(&ele[a].label,&ele[a].value,&ele[a].lineno);a++);
  qsort(ele,b,sizeof(struct info),compare);
  printf("       Cross reference:\n");
  printf("      symbol value hexa  line\n");
  for (a=0;a<b;a++)
   printf("%12.12s %5d %4hx %5u\n",ele[a].label,ele[a].value,
          (unsigned short)ele[a].value,ele[a].lineno);
  free(ele);
  a=0;
}
asm_close();
return a;
}
