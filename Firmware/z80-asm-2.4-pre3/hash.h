/* HASHING TABLE */

#ifndef __HASH_H
#define __HASH_H

#include "z80-global"

/* return number of used hash table entries */

extern unsigned table_entries(void);

/* adds item to hash table */

extern int add_to_table(char *key, int value, unsigned lineno, bool copy);

extern void update_last_added_entry(int value, char *txt, bool unique);

/* if 1 then returns next entry in hash table, else 0 and table scanned */

extern int  next_table_entry(char **key, int *value, unsigned *lineno);

/* tests if label is in table, returns true/false */
/* refresh global variable last_ele */

extern bool reaccess_label(char *key, unsigned lineno);

/* tests if label is in table  returns 1 or 0 */
/* stores address of label in *value if it is in table and value!=0 */

extern int is_in_table(char *key, unsigned len, int *value, unsigned lineno);

extern bool last_label_reusable(void);

/* initializes hash table */

extern void hash_table_init(void);

/* removes hash table from memory */

extern void free_hash_table(void);

#endif
