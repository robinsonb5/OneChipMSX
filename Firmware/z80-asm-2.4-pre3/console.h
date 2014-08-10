/* CONSOLE INTERFACE */

#ifndef __CONSOLE_H
#define __CONSOLE_H

extern int console_ok;

extern void c_init(char a);
extern void c_shutdown(void);
extern void c_cls(void);
extern void c_print(const char *text);
extern void c_goto(int x, int y);
extern unsigned char c_getkey(void);
extern void c_clear(int x1,int y1,int x2, int y2);
extern int c_kbhit(void);
extern void c_cursor(int c);
extern void c_bell(void);
extern void c_setcolor(char a);
extern void c_refresh(void);

#endif
