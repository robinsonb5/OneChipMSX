#ifndef TEXTBUFFER_H
#define TEXTBUFFER_H

#define VGA_CHARBUFFER_BASE 0xffffe000

void ClearTextBuffer();

int tb_putchar(int c);
int tb_puts(const char *str);

#endif
