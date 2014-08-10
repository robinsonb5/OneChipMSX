/* HASHING TABLE */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "asm.h"
#include "expression.h"

#define TABLE_SIZE (1<<13)  /* size of hash table (must be power of 2) */
#define MAGICKA_KONSTANTA  13  /* proof MJ's magic constant */

struct info{
int address;   /* label address/value */
char *pointer; /* pointer to a label name */
unsigned lineno;    /* defining line no */
char *expr;    /* to evaluate equations */
bool  flag;    /* to detect cycles in evaluation */
bool  uniq;    /* to allow redefinitions */
};

struct table_type{                    /* HASHING TABLE */
unsigned char count;
struct info *element;
};

static struct info *last_ele, *just_ele;  /* last for write, just for read-access */
static struct table_type entry[TABLE_SIZE];
static unsigned  counter;


unsigned table_entries(void)
{
   return  counter;
}


/* hashing function */

static unsigned
hashl(char *slovo, unsigned len)
{
unsigned b=0;

while(len--)
{
   b+=slovo[len];
   b*=MAGICKA_KONSTANTA;
}
return b&(TABLE_SIZE-1);
}


static unsigned
hash(char *slovo)
{
return hashl(slovo,strlen(slovo));
}


int  next_table_entry(char **key, int *value, unsigned *lineno)
{
static unsigned  a=0, c=0;
while (c >= entry[a].count)
{  c=0;  if (++a == TABLE_SIZE)  return a=0; }
*key=entry[a].element[c].pointer;
*value=entry[a].element[c].address;
*lineno=entry[a].element[c].lineno;
c++;
return 1;
}


/* adds item to hash table */
/* returns nullpointer in case of error else the table address of label */

int
add_to_table(char *key,int value,unsigned lineno, bool copy)
{
unsigned a,c;
a=hash(key);
c=entry[a].count;
if (!(c&c-1))
{ entry[a].element=realloc(entry[a].element,(2*c+!c)*sizeof(struct info));
if (!entry[a].element)return 0; }
if (copy)
{  unsigned len=strlen(key);
   char *temp= malloc(len+1);
   if (!temp) return 0;
   memcpy(temp,key,len);
   temp[len]=0;
   entry[a].element[c].pointer=temp;
}
else
   entry[a].element[c].pointer=key;
last_ele= &entry[a].element[c];
last_ele->address=value;
last_ele->lineno=lineno;
last_ele->expr= NULL;
last_ele->uniq= 1;
counter++;
if (!++entry[a].count)
{fprintf(stderr,"Error: internal hash counter overflow\n"); return 0;}
return 1;
}


void update_last_added_entry(int value, char *txt, bool unique)
{
   last_ele->address=value;
   last_ele->expr=txt;
   last_ele->flag=0;
   last_ele->uniq=unique;
}

/* tests if label is in table, returns 1 or 0 */
/* refresh global variable last_ele */

bool
reaccess_label(char *key, unsigned lineno)
{
int a,b;

a=hash(key);
if (!entry[a].count) return 0;
for (b=0;b<entry[a].count;b++)
   if (!strcmp(key,entry[a].element[b].pointer) &&
       (!lineno || entry[a].element[b].lineno == lineno))
   {  last_ele= &entry[a].element[b];
      return  1;
   }
return 0;
}


/* tests if label is in table, returns 1 or 0 */
/* stores address of label in *value if it is in table and value!=0 */

int 
is_in_table(char *key, unsigned len, int *value, unsigned lineno)
{
int a,b,c;

/* if not in table  we need address in first pass  to check JR distance */
/* but for EQU we need its true value for possible range check later */
a=len?hashl(key,len):hash(key);
if (!entry[a].count) return 0;
c= -1;
for (b=0;b<entry[a].count;b++)
   if (len && !strncmp(key,entry[a].element[b].pointer,len)  ||
       !len && !strcmp(key,entry[a].element[b].pointer) )
   {  if (!entry[a].element[b].uniq && lineno &&
          entry[a].element[b].lineno > lineno)
         break;
      c=b;
      if (entry[a].element[b].uniq || !entry[a].element[b].lineno || !lineno)
         break;
   }
if (c < 0) return 0;
b=c;
just_ele= &entry[a].element[b];
if (value)
{  if (!just_ele->expr)
      *value=just_ele->address;
   else if (compile_pass() != 1)
   {  if (just_ele->flag&1)
         return 0;
      just_ele->flag |= 1;
      if (parse_expr(just_ele->expr, value, lineno))
         return 0;
      just_ele= &entry[a].element[b];
      just_ele->flag &= ~1;
      just_ele->address= *value;
      free(just_ele->expr);
      just_ele->expr=NULL;
   }
   else
      return 0;
}
return 1;
}


bool
last_label_reusable(void)
{
   return  !just_ele->uniq;
}


/* initializes hash table */

void 
hash_table_init(void)
{
int a;

for (a=0;a<TABLE_SIZE;a++)
 {
 entry[a].count=0;
 entry[a].element=NULL;
 }
counter=0;
}


/* removes hash table from memory */

void
free_hash_table(void)
{
int a,b;
for (a=0;a<TABLE_SIZE;a++)
 {
 for (b=0;b<entry[a].count;b++)
 { free(entry[a].element[b].pointer);
   free(entry[a].element[b].expr);
 }
 free(entry[a].element);
 }
}
