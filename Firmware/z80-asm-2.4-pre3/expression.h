#ifndef __EXPR_H
#define __EXPR_H

#include "z80-global" 

extern int test_label(char *txt);
extern int test_number(char *txt,int *value);
extern char * resolve_current_pc_and_store(char *txt);
extern int parse_expr(char *txt, int* value, unsigned lineno);

#endif
