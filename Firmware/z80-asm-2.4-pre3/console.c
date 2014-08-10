/* CONSOLE INTERFACE */

#include <ctype.h>
#include <termios.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#ifdef DOS
 #include <conio.h>
#endif


#include "console_token"

/* BOTH COORDINATES START AT 0 */

int console_ok=1;
struct termios term_setting;
/***********************************************
typedef unsigned char   cc_t;
typedef unsigned int    speed_t;
typedef unsigned int    tcflag_t;

#define NCCS 32
struct termios
  {
    tcflag_t c_iflag;       // input mode flags
    tcflag_t c_oflag;       // output mode flags
    tcflag_t c_cflag;       // control mode flags
    tcflag_t c_lflag;       // local mode flags
    cc_t c_line;            // line discipline
    cc_t c_cc[NCCS];        // control characters
    speed_t c_ispeed;       // input speed
    speed_t c_ospeed;       // output speed
  };
***************************************************/
static int  fd_in, fd_out;

void
c_refresh(void)
{
#ifdef UNIX
 fflush(stdout);
#endif
}


/* move cursor to [x,y] */
void
c_goto(int x,int y)
{
#ifdef UNIX
 printf("\033[%u;%uH",y+1,x+1);
 /* c_refresh(); */
#else
 gotoxy(x+1,y+1);
#endif
}


void
c_setcolor(char a)
/* accepts: 0-9,a-f,A-F */
/* sets foreground color */
{
#ifdef UNIX
 a=tolower(a);
 if (a>15)
 {
  if (a>='a'&&a<='f')a+=10-'a';
  else 
  { 
   if(a>='0'&&a<='9')a-='0';
   else return;
  }
 }
 if (a>7)printf("\033[1;%dm",30+a-8);
 else printf("\033[0;%dm",30+a);
#else
 textcolor(a);
#endif
}


/* print on the cursor position */
void
c_print(const char * text)
{
#ifdef UNIX
 printf("%s",text);   /* text can contain escape sequencies, % and so on */
 c_refresh();
#else
 cputs(text);
#endif
}


/* clears the screen */
void
c_cls(void)
{
#ifdef UNIX
 printf("\033[2J");
 c_refresh();
#else
 clrscr();
#endif
}


/* clears rectangle on the screen */
/* presumtions: x2>=x1 && y2>=y1 */
void
c_clear(int x1,int y1,int x2,int y2)
{
 static char line[81];
 int y;
 
 for (y=0;y<x2-x1+1;y++)
  line[y]=' ';
 line[y]=0;
 for(y=y1;y<=y2;y++)
 {
  c_goto(x1,y);c_print(line);
 }
 c_refresh();
}


/* returns 1, if some key was pressed, otherwise returns 0 */
int
c_kbhit(void)
{
#ifdef UNIX
 fd_set rfds;
 struct timeval tv;
 int retval;

 FD_ZERO(&rfds);
 FD_SET(fd_in,&rfds);
 tv.tv_sec=0;
 tv.tv_usec=0;

 retval=select(1,&rfds,NULL,NULL,&tv);
 if (retval)return 1;
 else return 0;
#else
 return kbhit();
#endif
}


void
c_cursor(int type)
{
 switch (type)
 {
  case C_NORMAL:
#ifdef UNIX
  printf("\033[?25h");
#else
 _setcursortype(_NORMALCURSOR);
#endif
  break;

  case C_HIDE:
#ifdef UNIX
  printf("\033[?25l");
#else
 _setcursortype(_NOCURSOR);
#endif
  break;
 }
 c_refresh();
}


/* ring the bell */
void
c_bell(void)
{
 printf("\007");
 c_refresh();
}


/* initialize console */
void
c_init(char a)
{
#ifdef UNIX
 struct termios t;
 fd_in=fileno(stdin);
 fd_out=fileno(stdout);

 fflush(stdin);
 console_ok=0;
 if (tcgetattr(fd_in,&term_setting))
    exit((printf("Error: can't get terminal attributes\n"), 3));
 tcgetattr(fd_in,&t);
/*********
printf("c_iflag=%8x  c_oflag=%8x  c_cflag=%8x  c_lflag=%8x\n\
c_line=%2x  c_ispeed=%8x  c_ospeed=%8x\n",
t.c_iflag,t.c_oflag,t.c_cflag,t.c_lflag,t.c_line,t.c_ispeed,t.c_ospeed);
for (fd_in=NCCS;fd_in--;)
  printf("c_cc[%d]=%2x  ",fd_in,t.c_cc[fd_in]);
**********/
/**  cfmakeraw(&t);  // getchar may not block !!  **/
 t.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP
                 |INLCR|IGNCR|ICRNL|IXON);
 t.c_oflag &= ~OPOST;
 t.c_lflag &= ~(ECHO|ICANON|ISIG|IEXTEN);
 t.c_cflag &= ~(CSIZE|PARENB);
 t.c_cflag |= CS8;

 t.c_oflag |= OPOST;
 t.c_cc[VMIN]=1;  /* get at least 1 character if read */
 if (tcsetattr(fd_in, TCSANOW, &t))
    exit((printf("Error: can't set terminal attributes\n"), 3));
 printf("\033[=3h"); /* choose color 80x25 resolution */
 printf("\033[%dm",40+(a&7));
 c_cls();
 printf("\033[;H");
 c_refresh();
#else
 c_cls();
#endif
}


/* waits for a key, returns keycode */
unsigned char
c_getkey(void)
{
 int k;
#ifdef UNIX
 while ((k=getchar()) == EOF); /* getchar may not block in non canonical mode */
 switch(k)
 {
  case 9:
  return K_TAB;

  case 13:
  return K_ENTER;

  case 27:
   switch(getchar())
   {
    case 91:
    switch(getchar())
    {
     case 65:
     return K_UP;
     
     case 66:
     return K_DOWN;

     case 67:
     return K_RIGHT;

     case 68:
     return K_LEFT;
    }
   }
  return K_ESCAPE;

  case 127:
  return K_BACKSPACE;

  default:
  return k;
 }
#else
 while ((k=getch()) == EOF); /* getch may not block in non canonical mode */
 switch (k)
 {
  case 9:
  return K_TAB;
     
  case 8:
  return K_BACKSPACE;
     
  case 13:
  return K_ENTER;

  case 27:
  return K_ESCAPE;

  case 0:
  switch(getch())
  {
   case 72:
   return K_UP;
   
   case 80:
   return K_DOWN;
   break;
   
   case 75:
   return K_LEFT;
   
   case 77:
   return K_RIGHT;
  }
  break;
  
  default:
  return k;
 }
#endif

return 0;  /* some C compilers are shitty and say "control reaches end of non void function" */
}


/* close console */
void
c_shutdown(void)
{
#ifdef UNIX
 tcsetattr(fd_in,TCSANOW,&term_setting);
#endif
 c_cursor(C_NORMAL);
#ifdef UNIX
 printf("\033[%dm",39); /* default foreground */
 printf("\033[%dm",49); /* default background */
#endif
 c_cls();
#ifdef UNIX
 printf("\033[;H");
 c_refresh();
#endif
 console_ok=1;
}
