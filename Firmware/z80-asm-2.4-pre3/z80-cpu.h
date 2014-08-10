#include "z80-global"

#ifndef __Z80CPU
#define __Z80CPU

extern _uchar F,A,B,C,D,E,H,L;
extern _uchar F_,A_,B_,C_,D_,E_,H_,L_;
extern _uchar IXl,IXh,IYl,IYh;
extern _uchar I,R,IM;
extern _ushort PC,SP,IX,IY;
extern _uchar DATA;
extern _ushort ADDRESS;
extern bit  IFF0;      /* internal EI-flipflop */
extern bit  IFF1,IFF2;
extern bit  IFF3;      /* NMI has occured */

/*************************
   rd       cpu wants to read data
   wr       cpu wants to write data
   iorq     cpu signals to access external IO
   mreq     cpu signals to access memory 
   m1       machine cycle one  (together with iorq acknowledges interrupt)
   inter    maskable interrupt pending
   halt     cpu in halt instruction
   wait     cpu in wait state
   reset    cpu reset requested
   rfsh     cpu signals memory refresh (if mreq is set)
   busrq    external hardware requests bus control
   busack   cpu acknowledges bus control
*********************/
enum cpu_control_pin { rd, wr, iorq, mreq, m1, inter, halt, wait, reset, rfsh,
                       busrq, busack };
extern const bit cpu_pin[NO_CPU_CONTROL_PINS];

extern const unsigned long ticks, cycles;
extern bool cpu_is_in_disassemble, cpu_is_in_x_mode;
extern void  set_cpu_pin(unsigned p, bit level);
extern void wait_tics(unsigned duration);
extern void set_tics(unsigned long t);
extern void acknowledge_bus_request(void);

#endif
